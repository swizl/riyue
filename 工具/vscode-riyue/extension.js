// 日月语言 VS Code 扩展（阶段 9.5 生态）
//
// 组成：
//   1) 语法高亮（syntaxes/riyue.tmLanguage.json）+ 语言配置 + 代码片段（snippets/riyue.json）
//   2) LSP 客户端：编译 `工具/LSP服务端.心` → `构建/LSP服务端.exe`，以 stdio 启动并自定义
//      Content-Length 帧（不依赖 vscode-languageclient / npm 依赖）
//   3) 任务命令：编译当前文件 / 编译并运行 / 格式化文档
//   4) AI 助手入口：编译 `工具/AI助手.心` → `构建/AI助手.exe`（→ 标准库/AI.心），
//      生成插入光标处、解释新开文档
//
// 说明：服务端已实现 completion / definition / hover / references / documentSymbol /
//       rename / publishDiagnostics；本客户端按需转发，其余一律返回 null（不报错）。

const vscode = require('vscode');
const cp = require('child_process');
const fs = require('fs');
const path = require('path');

let client = null;
let diagnostics = null;
let channel = null;
let started = false;

function config() {
    return vscode.workspace.getConfiguration('日月');
}

function projectRoot() {
    const configured = config().get('项目根');
    if (configured && String(configured).trim() !== '') {
        return String(configured);
    }
    const folders = vscode.workspace.workspaceFolders;
    if (folders && folders.length > 0) {
        return folders[0].uri.fsPath;
    }
    return process.cwd();
}

function compilerPath() {
    const p = config().get('编译器路径') || '日月.exe';
    return path.isAbsolute(p) ? p : path.join(projectRoot(), p);
}

function toolScript(name) {
    return path.join(projectRoot(), '工具', name);
}

function toRange(r) {
    return new vscode.Range(
        r.start.line, r.start.character,
        r.end.line, r.end.character
    );
}

function toLocation(loc) {
    return new vscode.Location(vscode.Uri.parse(loc.uri), toRange(loc.range));
}

function severityOf(code) {
    if (code === 1) { return vscode.DiagnosticSeverity.Error; }
    if (code === 2) { return vscode.DiagnosticSeverity.Warning; }
    if (code === 3) { return vscode.DiagnosticSeverity.Information; }
    return vscode.DiagnosticSeverity.Hint;
}

class LspClient {
    constructor() {
        this.proc = null;
        this.buf = Buffer.alloc(0);
        this.nextId = 1;
        this.pending = new Map();
    }

    async start(serverExe) {
        this.stop();
        if (!fs.existsSync(serverExe)) {
            channel.appendLine('未找到语言服务: ' + serverExe);
            return;
        }
        this.proc = cp.spawn(serverExe, [], {
            cwd: projectRoot(),
            windowsHide: true,
            stdio: ['pipe', 'pipe', 'pipe']
        });
        this.proc.stdout.on('data', (d) => this.onData(d));
        this.proc.stderr.on('data', (d) => channel.append(d.toString()));
        this.proc.on('error', (e) => channel.appendLine('语言服务启动失败: ' + e.message));
        this.proc.on('exit', (code) => {
            channel.appendLine('语言服务已退出: ' + code);
            this.proc = null;
        });
        await this.request('initialize', {
            processId: process.pid,
            rootUri: vscode.Uri.file(projectRoot()).toString(),
            capabilities: {
                textDocument: {
                    completion: {},
                    hover: {},
                    definition: {},
                    references: {},
                    documentSymbol: {},
                    rename: {},
                    publishDiagnostics: {}
                }
            }
        });
        this.notify('initialized', {});
    }

    stop() {
        const proc = this.proc;
        this.proc = null;
        if (!proc) { return; }
        try { proc.stdin.write(this.frame({ jsonrpc: '2.0', method: 'shutdown', params: {} })); } catch (e) { /* 进程可能已退出 */ }
        try { proc.stdin.write(this.frame({ jsonrpc: '2.0', method: 'exit', params: {} })); } catch (e) { /* 同上 */ }
        try { proc.kill(); } catch (e) { /* 同上 */ }
        this.buf = Buffer.alloc(0);
        this.pending.clear();
    }

    onData(chunk) {
        this.buf = Buffer.concat([this.buf, chunk]);
        for (;;) {
            const sep = this.buf.indexOf('\r\n\r\n');
            if (sep < 0) { return; }
            const header = this.buf.slice(0, sep).toString('ascii');
            const m = /Content-Length:\s*(\d+)/i.exec(header);
            if (!m) {
                this.buf = this.buf.slice(sep + 4);
                continue;
            }
            const len = parseInt(m[1], 10);
            if (this.buf.length < sep + 4 + len) { return; }
            const body = this.buf.slice(sep + 4, sep + 4 + len).toString('utf8');
            this.buf = this.buf.slice(sep + 4 + len);
            let msg = null;
            try { msg = JSON.parse(body); } catch (e) { channel.appendLine('语言服务报文解析失败'); continue; }
            this.onMessage(msg);
        }
    }

    onMessage(msg) {
        if (msg.id !== undefined && this.pending.has(msg.id)) {
            const done = this.pending.get(msg.id);
            this.pending.delete(msg.id);
            done(msg.error ? null : (msg.result === undefined ? null : msg.result));
            return;
        }
        if (msg.method === 'textDocument/publishDiagnostics' && diagnostics) {
            const uri = vscode.Uri.parse(msg.params.uri);
            const items = (msg.params.diagnostics || []).map((d) => {
                const diag = new vscode.Diagnostic(toRange(d.range), d.message || '', severityOf(d.severity));
                diag.source = '日月';
                return diag;
            });
            diagnostics.set(uri, items);
            return;
        }
        if (msg.method === 'window/logMessage' && msg.params) {
            channel.appendLine(String(msg.params.message || ''));
        }
    }

    frame(obj) {
        const body = Buffer.from(JSON.stringify(obj), 'utf8');
        return Buffer.concat([
            Buffer.from('Content-Length: ' + body.length + '\r\n\r\n', 'ascii'),
            body
        ]);
    }

    request(method, params) {
        return new Promise((resolve) => {
            if (!this.proc || !this.proc.stdin.writable) { resolve(null); return; }
            const id = this.nextId++;
            this.pending.set(id, resolve);
            try {
                this.proc.stdin.write(this.frame({ jsonrpc: '2.0', id: id, method: method, params: params }));
            } catch (e) {
                this.pending.delete(id);
                resolve(null);
                return;
            }
            setTimeout(() => {
                if (this.pending.has(id)) {
                    this.pending.delete(id);
                    resolve(null);
                }
            }, 5000);
        });
    }

    notify(method, params) {
        if (!this.proc || !this.proc.stdin.writable) { return; }
        try {
            this.proc.stdin.write(this.frame({ jsonrpc: '2.0', method: method, params: params }));
        } catch (e) { /* 进程已退出 */ }
    }
}

function documentParams(doc) {
    return { textDocument: { uri: doc.uri.toString() } };
}

function activate(context) {
    channel = vscode.window.createOutputChannel('日月');
    diagnostics = vscode.languages.createDiagnosticCollection('riyue');
    context.subscriptions.push(channel, diagnostics);

    const run = (exe, args) => new Promise((resolve) => {
        const proc = cp.spawn(exe, args, { cwd: projectRoot(), windowsHide: true });
        let out = '';
        let err = '';
        proc.stdout.on('data', (d) => { out += d.toString(); });
        proc.stderr.on('data', (d) => { err += d.toString(); });
        proc.on('error', (e) => resolve({ code: -1, out: out, err: out + e.message }));
        proc.on('exit', (code) => resolve({ code: code === null ? -1 : code, out: out, err: err }));
    });

    // 工具脚本 → 构建/<名>.exe（源文件较新或产物缺失时重新编译；失败返回 null）
    const ensureTool = async (sourceName, outputName) => {
        const source = toolScript(sourceName + '.心');
        const dir = path.join(projectRoot(), '构建');
        if (!fs.existsSync(dir)) { fs.mkdirSync(dir, { recursive: true }); }
        const exe = path.join(dir, outputName + '.exe');
        const stale = !fs.existsSync(exe) ||
            (fs.existsSync(source) && fs.statSync(source).mtimeMs > fs.statSync(exe).mtimeMs);
        if (!stale) { return exe; }
        channel.appendLine('正在构建: ' + source);
        const result = await run(compilerPath(), [source, path.join(dir, outputName)]);
        if (result.code !== 0 || !fs.existsSync(exe)) {
            channel.appendLine(result.err || result.out || ('构建失败（退出码 ' + result.code + '）'));
            channel.show(true);
            return null;
        }
        return exe;
    };

    client = new LspClient();

    const ensureStarted = async () => {
        if (started) { return; }
        started = true;
        const serverExe = await ensureTool('LSP服务端', 'LSP服务端');
        if (!serverExe) {
            started = false;
            vscode.window.showErrorMessage('日月语言服务构建失败，详见输出面板。');
            return;
        }
        await client.start(serverExe);
    };

    const isRiyue = (doc) => doc && doc.languageId === 'riyue';

    if (vscode.window.activeTextEditor && isRiyue(vscode.window.activeTextEditor.document)) {
        ensureStarted();
    }

    context.subscriptions.push(vscode.workspace.onDidOpenTextDocument(async (doc) => {
        if (!isRiyue(doc)) { return; }
        await ensureStarted();
        client.notify('textDocument/didOpen', {
            textDocument: {
                uri: doc.uri.toString(),
                languageId: 'riyue',
                version: doc.version,
                text: doc.getText()
            }
        });
    }));

    context.subscriptions.push(vscode.workspace.onDidChangeTextDocument((e) => {
        if (!isRiyue(e.document)) { return; }
        client.notify('textDocument/didChange', {
            textDocument: { uri: e.document.uri.toString(), version: e.document.version },
            contentChanges: [{ text: e.document.getText() }]
        });
    }));

    context.subscriptions.push(vscode.workspace.onDidCloseTextDocument((doc) => {
        if (!isRiyue(doc)) { return; }
        client.notify('textDocument/didClose', { textDocument: { uri: doc.uri.toString() } });
        diagnostics.delete(doc.uri);
    }));

    // ---- LSP 能力转发 ----

    context.subscriptions.push(vscode.languages.registerCompletionItemProvider('riyue', {
        async provideCompletionItems(doc, position) {
            const result = await client.request('textDocument/completion', {
                textDocument: { uri: doc.uri.toString() },
                position: { line: position.line, character: position.character }
            });
            if (!result || !Array.isArray(result)) { return null; }
            return result.map((item) => {
                const label = (item && typeof item.label === 'object') ? item.label.label : String(item.label || '');
                return new vscode.CompletionItem(label, vscode.CompletionItemKind.Text);
            });
        }
    }));

    context.subscriptions.push(vscode.languages.registerDefinitionProvider('riyue', {
        async provideDefinition(doc, position) {
            const result = await client.request('textDocument/definition', {
                textDocument: { uri: doc.uri.toString() },
                position: { line: position.line, character: position.character }
            });
            if (!result) { return null; }
            return Array.isArray(result) ? result.map(toLocation) : toLocation(result);
        }
    }));

    context.subscriptions.push(vscode.languages.registerHoverProvider('riyue', {
        async provideHover(doc, position) {
            const result = await client.request('textDocument/hover', {
                textDocument: { uri: doc.uri.toString() },
                position: { line: position.line, character: position.character }
            });
            if (!result || !result.contents) { return null; }
            const contents = result.contents;
            if (typeof contents === 'string') { return new vscode.Hover(contents); }
            if (Array.isArray(contents)) {
                return new vscode.Hover(contents.map((c) => (typeof c === 'string' ? c : (c.value || ''))));
            }
            return new vscode.Hover(contents.value || '');
        }
    }));

    context.subscriptions.push(vscode.languages.registerReferenceProvider('riyue', {
        async provideReferences(doc, position) {
            const result = await client.request('textDocument/references', {
                textDocument: { uri: doc.uri.toString() },
                position: { line: position.line, character: position.character },
                context: { includeDeclaration: true }
            });
            if (!result || !Array.isArray(result)) { return null; }
            return result.map(toLocation);
        }
    }));

    context.subscriptions.push(vscode.languages.registerDocumentSymbolProvider('riyue', {
        async provideDocumentSymbols(doc) {
            const result = await client.request('textDocument/documentSymbol', documentParams(doc));
            if (!result || !Array.isArray(result)) { return null; }
            return result.map((s) => new vscode.SymbolInformation(
                s.name,
                s.kind,
                '',
                toLocation(s.location)
            ));
        }
    }));

    context.subscriptions.push(vscode.languages.registerRenameProvider('riyue', {
        async provideRenameEdits(doc, position, newName) {
            const result = await client.request('textDocument/rename', {
                textDocument: { uri: doc.uri.toString() },
                position: { line: position.line, character: position.character },
                newName: newName
            });
            if (!result || !result.changes) { return null; }
            const edit = new vscode.WorkspaceEdit();
            for (const uri of Object.keys(result.changes)) {
                const list = result.changes[uri] || [];
                for (const change of list) {
                    edit.replace(vscode.Uri.parse(uri), toRange(change.range), change.newText);
                }
            }
            return edit;
        }
    }));

    // ---- 任务与 AI 命令 ----

    const activeFile = () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor || !isRiyue(editor.document)) {
            vscode.window.showErrorMessage('请先打开一个日月（.心）源文件。');
            return null;
        }
        return editor;
    };

    const build = async (file) => {
        const outDir = path.join(projectRoot(), '构建');
        if (!fs.existsSync(outDir)) { fs.mkdirSync(outDir, { recursive: true }); }
        const prefix = path.join(outDir, path.basename(file, '.心'));
        const result = await run(compilerPath(), [file, prefix]);
        if (result.code !== 0) {
            channel.appendLine(result.err || result.out || ('编译失败（退出码 ' + result.code + '）'));
            channel.show(true);
            vscode.window.showErrorMessage('日月编译失败，详见输出面板。');
            return null;
        }
        return prefix + '.exe';
    };

    context.subscriptions.push(vscode.commands.registerCommand('riyue.compile', async () => {
        const editor = activeFile();
        if (!editor) { return; }
        const exe = await build(editor.document.uri.fsPath);
        if (exe) { vscode.window.showInformationMessage('编译成功: ' + exe); }
    }));

    context.subscriptions.push(vscode.commands.registerCommand('riyue.run', async () => {
        const editor = activeFile();
        if (!editor) { return; }
        const exe = await build(editor.document.uri.fsPath);
        if (!exe) { return; }
        const terminal = vscode.window.createTerminal('日月运行');
        terminal.show();
        terminal.sendText('"' + exe + '"');
    }));

    context.subscriptions.push(vscode.commands.registerCommand('riyue.format', async () => {
        const editor = activeFile();
        if (!editor) { return; }
        if (editor.document.isDirty) { await editor.document.save(); }
        const formatter = await ensureTool('统一命令行', '统一命令行');
        if (!formatter) { return; }
        const result = await run(formatter, ['格式', editor.document.uri.fsPath]);
        if (result.code !== 0) {
            channel.appendLine(result.out + result.err);
            channel.show(true);
            vscode.window.showErrorMessage('格式化失败，详见输出面板。');
            return;
        }
        await vscode.commands.executeCommand('workbench.action.files.revert');
        vscode.window.showInformationMessage('格式化完成。');
    }));

    const aiSettings = () => {
        const endpoint = String(config().get('AI端点') || '');
        if (endpoint === '') {
            vscode.window.showErrorMessage('请先在设置中配置「日月.AI端点」（OpenAI 兼容 chat completions 地址）。');
            return null;
        }
        return {
            endpoint: endpoint,
            key: String(config().get('AI密钥') || ''),
            model: String(config().get('AI模型') || 'gpt-4o-mini')
        };
    };

    const aiRun = async (args) => {
        const helper = await ensureTool('AI助手', 'AI助手');
        if (!helper) { return null; }
        const result = await run(helper, args);
        if (result.code !== 0 || result.out.trim() === '') {
            channel.appendLine(result.out + result.err);
            channel.show(true);
            vscode.window.showErrorMessage('AI 调用未返回内容（详见输出面板）。');
            return null;
        }
        return result.out;
    };

    context.subscriptions.push(vscode.commands.registerCommand('riyue.aiGenerate', async () => {
        const settings = aiSettings();
        if (!settings) { return; }
        const editor = activeFile();
        if (!editor) { return; }
        const requirement = await vscode.window.showInputBox({
            prompt: '描述要生成的日月代码（只输出代码）',
            placeHolder: '例如：读取文件并统计行数'
        });
        if (!requirement) { return; }
        const output = await aiRun(['生成', settings.endpoint, settings.key, settings.model, requirement]);
        if (!output) { return; }
        await editor.edit((builder) => builder.insert(editor.selection.active, output));
    }));

    context.subscriptions.push(vscode.commands.registerCommand('riyue.aiExplain', async () => {
        const settings = aiSettings();
        if (!settings) { return; }
        const editor = activeFile();
        if (!editor) { return; }
        const buffer = path.join(projectRoot(), '构建');
        if (!fs.existsSync(buffer)) { fs.mkdirSync(buffer, { recursive: true }); }
        let target = editor.document.uri.fsPath;
        let temp = null;
        if (!editor.selection.isEmpty) {
            temp = path.join(buffer, '.AI解释片段.心');
            fs.writeFileSync(temp, editor.document.getText(editor.selection), 'utf8');
            target = temp;
        }
        const output = await aiRun(['解释', settings.endpoint, settings.key, settings.model, target]);
        if (temp && fs.existsSync(temp)) { fs.unlinkSync(temp); }
        if (!output) { return; }
        const doc = await vscode.workspace.openTextDocument({ content: output, language: 'markdown' });
        await vscode.window.showTextDocument(doc, { preview: false });
    }));

    context.subscriptions.push(vscode.commands.registerCommand('riyue.restartServer', async () => {
        started = false;
        client.stop();
        diagnostics.clear();
        await ensureStarted();
        vscode.window.showInformationMessage('日月语言服务已重启。');
    }));
}

function deactivate() {
    if (client) { client.stop(); }
    if (diagnostics) { diagnostics.dispose(); }
}

module.exports = { activate, deactivate };
