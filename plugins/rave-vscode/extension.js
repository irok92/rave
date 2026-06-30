"use strict";

const vscode = require("vscode");
const {
    LanguageClient,
    TransportKind,
    State,
} = require("vscode-languageclient/node");

let client;

async function activate(context) {
    const serverPath = vscode.workspace
        .getConfiguration("rave.server")
        .get("path", "rave.exe");

    const serverOptions = {
        command: serverPath,
        args: [],
        transport: TransportKind.stdio,
    };

    const clientOptions = {
        documentSelector: [{ scheme: "file", language: "rave" }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher("**/*.rave"),
        },
        initializationOptions: {},
        outputChannelName: "Rave LSP",
        traceOutputChannelName: "Rave LSP Trace",
        markdown: {
            isTrusted: true,
        },
        // Advertise full client-side capabilities so
        // the server enables all features.
        capabilities: {
            textDocument: {
                synchronization: {
                    dynamicRegistration: true,
                    willSave: true,
                    willSaveWaitUntil: true,
                    didSave: true,
                },
                completion: {
                    dynamicRegistration: true,
                    completionItem: {
                        snippetSupport: true,
                        commitCharactersSupport: true,
                        documentationFormat: ["markdown", "plaintext"],
                        deprecatedSupport: true,
                        preselectSupport: true,
                        tagSupport: { valueSet: [1] },
                        insertReplaceSupport: true,
                        resolveSupport: { properties: ["documentation", "detail", "additionalTextEdits"] },
                        labelDetailsSupport: true,
                    },
                    completionItemKind: { valueSet: Array.from({ length: 25 }, (_, i) => i + 1) },
                    contextSupport: true,
                    insertTextMode: 2,
                    completionList: { itemDefaults: ["commitCharacters", "editRange", "insertTextFormat", "insertTextMode", "data"] },
                },
                hover: {
                    dynamicRegistration: true,
                    contentFormat: ["markdown", "plaintext"],
                },
                signatureHelp: {
                    dynamicRegistration: true,
                    signatureInformation: {
                        documentationFormat: ["markdown", "plaintext"],
                        parameterInformation: { labelOffsetSupport: true },
                        activeParameterSupport: true,
                    },
                    contextSupport: true,
                },
                definition: { dynamicRegistration: true, linkSupport: true },
                declaration: { dynamicRegistration: true, linkSupport: true },
                typeDefinition: { dynamicRegistration: true, linkSupport: true },
                implementation: { dynamicRegistration: true, linkSupport: true },
                references: { dynamicRegistration: true },
                documentHighlight: { dynamicRegistration: true },
                documentSymbol: {
                    dynamicRegistration: true,
                    hierarchicalDocumentSymbolSupport: true,
                    symbolKind: { valueSet: Array.from({ length: 26 }, (_, i) => i + 1) },
                    tagSupport: { valueSet: [1] },
                },
                codeAction: {
                    dynamicRegistration: true,
                    isPreferredSupport: true,
                    disabledSupport: true,
                    dataSupport: true,
                    resolveSupport: { properties: ["edit"] },
                    codeActionLiteralSupport: {
                        codeActionKind: { valueSet: ["quickfix", "refactor", "refactor.extract", "refactor.inline", "refactor.rewrite", "source", "source.organizeImports"] },
                    },
                    honorsChangeAnnotations: true,
                },
                codeLens: { dynamicRegistration: true },
                documentLink: { dynamicRegistration: true, tooltipSupport: true },
                colorProvider: { dynamicRegistration: true },
                formatting: { dynamicRegistration: true },
                rangeFormatting: { dynamicRegistration: true, rangesSupport: true },
                onTypeFormatting: { dynamicRegistration: true },
                rename: {
                    dynamicRegistration: true,
                    prepareSupport: true,
                    prepareSupportDefaultBehavior: 1,
                    honorsChangeAnnotations: true,
                },
                foldingRange: {
                    dynamicRegistration: true,
                    rangeLimit: 5000,
                    lineFoldingOnly: true,
                    foldingRangeKind: { valueSet: ["comment", "imports", "region"] },
                    foldingRange: { collapsedText: true },
                },
                selectionRange: { dynamicRegistration: true },
                linkedEditingRange: { dynamicRegistration: true },
                semanticTokens: {
                    dynamicRegistration: true,
                    requests: { full: { delta: true }, range: true },
                    tokenTypes: ["keyword", "type", "function", "variable", "string", "number", "comment", "operator", "parameter", "property", "enumMember", "event", "macro", "modifier", "decorator", "label"],
                    tokenModifiers: ["declaration", "definition", "readonly", "static", "deprecated", "abstract", "async", "documentation"],
                    formats: ["relative"],
                    overlappingTokenSupport: false,
                    multilineTokenSupport: true,
                    serverCancelSupport: true,
                    augmentsSyntaxTokens: true,
                },
                inlayHint: { dynamicRegistration: true, resolveSupport: { properties: ["tooltip", "textEdits", "label.tooltip", "label.location", "label.command"] } },
                inlineValue: { dynamicRegistration: true },
                inlineCompletion: { dynamicRegistration: true },
                publishDiagnostics: {
                    relatedInformation: true,
                    tagSupport: { valueSet: [1, 2] },
                    versionSupport: true,
                    codeDescriptionSupport: true,
                    dataSupport: true,
                },
                diagnostic: {
                    dynamicRegistration: true,
                    relatedDocumentSupport: false,
                },
                moniker: { dynamicRegistration: true },
                typeHierarchy: { dynamicRegistration: true },
            },
            workspace: {
                symbol: {
                    dynamicRegistration: true,
                    resolveSupport: { properties: ["location.range"] },
                    symbolKind: { valueSet: Array.from({ length: 26 }, (_, i) => i + 1) },
                    tagSupport: { valueSet: [1] },
                },
                workspaceFolders: true,
                configuration: true,
                didChangeConfiguration: { dynamicRegistration: true },
                didChangeWatchedFiles: { dynamicRegistration: true, relativePatternSupport: true },
                fileOperations: {
                    dynamicRegistration: true,
                    didCreate: true,
                    willCreate: true,
                    didRename: true,
                    willRename: true,
                    didDelete: true,
                    willDelete: true,
                },
                diagnostics: { refreshSupport: true },
                inlayHint: { refreshSupport: true },
                inlineValue: { refreshSupport: true },
                semanticTokens: { refreshSupport: true },
                codeLens: { refreshSupport: true },
            },
            window: {
                workDoneProgress: true,
                showMessage: {
                    messageActionItem: { additionalPropertiesSupport: true },
                },
            },
            general: {
                staleRequestSupport: { cancel: true, retryOnContentModified: [] },
                regularExpressions: { engine: "ECMAScript", version: "ES2024" },
            },
            notebookDocument: {
                synchronization: {
                    dynamicRegistration: true,
                    executionSummarySupport: true,
                },
            },
        },
    };

    client = new LanguageClient(
        "rave-lsp",
        "Rave Language Server",
        serverOptions,
        clientOptions
    );

    client.onDidChangeState((e) => {
        if (e.newState === State.Running) {
        }
    });

    try {
        await client.start();
        vscode.window.showInformationMessage("Rave LSP started");
    } catch (e) {
        vscode.window.showErrorMessage(`Rave LSP failed: ${e.message}`);
    }

    context.subscriptions.push(
        vscode.commands.registerCommand("rave.restartLanguageServer", async () => {
            vscode.window.showInformationMessage("Restarting Rave LSP...");
            if (client) {
                await client.stop();
                await client.start();
                vscode.window.showInformationMessage("Rave LSP restarted");
            }
        })
    );
}

function deactivate() {
    if (client) {
        return client.stop();
    }
}

module.exports = { activate, deactivate };
