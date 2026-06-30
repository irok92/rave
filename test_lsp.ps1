# test_lsp.ps1 — full LSP 3.18 scaffold test for rave-server
# Sends every request method and prints responses.
#
# Usage: pwsh test_lsp.ps1

param(
    [string]$Server = "$PSScriptRoot/build/bin/rave.exe"
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path $Server)) {
    Write-Host "ERROR: Server not found at $Server"
    exit 1
}

# ── Helpers ──────────────────────────────────────────────────────────

function Send-Message($sw, $msg) {
    $json = ($msg | ConvertTo-Json -Depth 10 -Compress)
    $hdr  = "Content-Length: $($json.Length)`r`n`r`n"
    $sw.Write($hdr + $json)
}

function Read-Response($sr) {
    $header = ""
    $cl = $null
    while ($true) {
        $ch = $sr.Read()
        if ($ch -lt 0) { return $null }
        $c = [char]$ch
        $header += $c
        if ($header -match "Content-Length: (\d+)\r\n\r\n$") {
            $cl = [int]$Matches[1]
            break
        }
        if ($header.Length -gt 256) { return $null }
    }
    if ($cl -le 0) { return $null }
    $body = New-Object char[] $cl
    $sr.Read($body, 0, $cl) | Out-Null
    -join $body
}

function Print-Step($num, $label) {
    Write-Host "`n[$num] $label" -ForegroundColor Yellow
}

function Print-Response($label, $rspJson) {
    if ($rspJson) {
        $pretty = try { ($rspJson | ConvertFrom-Json | ConvertTo-Json -Depth 10) } catch { $rspJson }
        Write-Host "   ← $label" -ForegroundColor Cyan
        foreach ($line in ($pretty -split "`n")) {
            Write-Host "      $line"
        }
    }
}

function Print-NoResponse {
    Write-Host "   ← (notification — no response)" -ForegroundColor DarkGray
}

# ── Common params ────────────────────────────────────────────────────

$docUri  = "file:///project/main.rave"
$docRef  = @{ uri = $docUri }
$pos     = @{ line = 2; character = 14 }

# ── Start server ─────────────────────────────────────────────────────

Write-Host "`n=== rave-server LSP scaffold test ===" -ForegroundColor Cyan
$pi = New-Object System.Diagnostics.ProcessStartInfo
$pi.FileName = $Server
$pi.RedirectStandardInput  = $true
$pi.RedirectStandardOutput = $true
$pi.RedirectStandardError  = $true
$pi.UseShellExecute = $false
$pi.CreateNoWindow  = $true

$proc = [System.Diagnostics.Process]::Start($pi)
$sw = $proc.StandardInput
$sr = $proc.StandardOutput
Start-Sleep -Milliseconds 100

# ═══════════════════════════════════════════════════════════════════════
# 1. Initialize
# ═══════════════════════════════════════════════════════════════════════
Print-Step 1 "initialize"
Send-Message $sw @{ jsonrpc="2.0"; id=1; method="initialize"
    params=@{ processId=$pid; rootUri="file:///project"
        capabilities=@{ textDocument=@{ hover=@{dynamicRegistration=$true}
            completion=@{dynamicRegistration=$true}
            definition=@{dynamicRegistration=$true}
            references=@{dynamicRegistration=$true}
            documentSymbol=@{dynamicRegistration=$true}
            formatting=@{dynamicRegistration=$true}
            rangeFormatting=@{dynamicRegistration=$true}
            rename=@{dynamicRegistration=$true}
            foldingRange=@{dynamicRegistration=$true}
            semanticTokens=@{dynamicRegistration=$true}
            codeAction=@{dynamicRegistration=$true}
            codeLens=@{dynamicRegistration=$true}
            documentLink=@{dynamicRegistration=$true}
            colorProvider=@{dynamicRegistration=$true}
            inlayHint=@{dynamicRegistration=$true}
            inlineCompletion=@{dynamicRegistration=$true}
            publishDiagnostics=@{dynamicRegistration=$true}
        }
        workspace=@{ symbol=@{dynamicRegistration=$true}
            configuration=$true; workspaceFolders=$true
            fileOperations=@{dynamicRegistration=$true}
        }
        notebookDocument=@{ sync=@{dynamicRegistration=$true} }
    }}
}
Print-Response "initialize" (Read-Response $sr)

# ═══════════════════════════════════════════════════════════════════════
# 2. initialized
# ═══════════════════════════════════════════════════════════════════════
Print-Step 2 "initialized"
Send-Message $sw @{ jsonrpc="2.0"; method="initialized"; params=@{} }
Print-NoResponse

# ═══════════════════════════════════════════════════════════════════════
# 3. didOpen
# ═══════════════════════════════════════════════════════════════════════
Print-Step 3 "textDocument/didOpen"
Send-Message $sw @{ jsonrpc="2.0"; method="textDocument/didOpen"
    params=@{ textDocument=@{ uri=$docUri; languageId="rave"; version=1;
        text="fn main() -> i32 { return 0; }" }
}}
Print-NoResponse

# ═══════════════════════════════════════════════════════════════════════
# 4–43: Language Features
# ═══════════════════════════════════════════════════════════════════════

$id=2
$tests = @(
    @{n=4;  l="hover";              m="textDocument/hover";              p=@{ textDocument=$docRef; position=$pos }},
    @{n=5;  l="completion";          m="textDocument/completion";          p=@{ textDocument=$docRef; position=$pos; context=@{triggerKind=1} }},
    @{n=6;  l="hover (again)";       m="textDocument/hover";              p=@{ textDocument=$docRef; position=$pos }},
    @{n=7;  l="signatureHelp";       m="textDocument/signatureHelp";       p=@{ textDocument=$docRef; position=$pos }},
    @{n=8;  l="definition";          m="textDocument/definition";          p=@{ textDocument=$docRef; position=$pos }},
    @{n=9;  l="declaration";         m="textDocument/declaration";         p=@{ textDocument=$docRef; position=$pos }},
    @{n=10; l="typeDefinition";      m="textDocument/typeDefinition";      p=@{ textDocument=$docRef; position=$pos }},
    @{n=11; l="implementation";      m="textDocument/implementation";      p=@{ textDocument=$docRef; position=$pos }},
    @{n=12; l="references";          m="textDocument/references";          p=@{ textDocument=$docRef; position=$pos; context=@{includeDeclaration=$true} }},
    @{n=13; l="documentHighlight";   m="textDocument/documentHighlight";   p=@{ textDocument=$docRef; position=$pos }},
    @{n=14; l="documentSymbol";      m="textDocument/documentSymbol";      p=@{ textDocument=$docRef }},
    @{n=15; l="codeAction";          m="textDocument/codeAction";          p=@{ textDocument=$docRef; range=@{start=$pos;end=$pos}; context=@{diagnostics=@()} }},
    @{n=16; l="codeLens";            m="textDocument/codeLens";            p=@{ textDocument=$docRef }},
    @{n=17; l="codeLens/resolve";    m="codeLens/resolve";                 p=@{ range=@{start=$pos;end=$pos}; data="test" }},
    @{n=18; l="documentLink";        m="textDocument/documentLink";        p=@{ textDocument=$docRef }},
    @{n=19; l="documentLink/resolve"; m="documentLink/resolve";            p=@{ range=@{start=$pos;end=$pos}; target="file:///test" }},
    @{n=20; l="documentColor";       m="textDocument/documentColor";       p=@{ textDocument=$docRef }},
    @{n=21; l="colorPresentation";   m="textDocument/colorPresentation";   p=@{ textDocument=$docRef; color=@{red=1;green=0;blue=0;alpha=1}; range=@{start=$pos;end=$pos} }},
    @{n=22; l="formatting";          m="textDocument/formatting";          p=@{ textDocument=$docRef; options=@{tabSize=4;insertSpaces=$true} }},
    @{n=23; l="rangeFormatting";     m="textDocument/rangeFormatting";     p=@{ textDocument=$docRef; range=@{start=$pos;end=$pos}; options=@{tabSize=4;insertSpaces=$true} }},
    @{n=24; l="rangesFormatting";    m="textDocument/rangesFormatting";    p=@{ textDocument=$docRef; ranges=@(@{start=$pos;end=$pos}); options=@{tabSize=4;insertSpaces=$true} }},
    @{n=25; l="onTypeFormatting";    m="textDocument/onTypeFormatting";    p=@{ textDocument=$docRef; position=$pos; ch="}" ; options=@{tabSize=4;insertSpaces=$true} }},
    @{n=26; l="rename";              m="textDocument/rename";              p=@{ textDocument=$docRef; position=$pos; newName="renamed" }},
    @{n=27; l="prepareRename";       m="textDocument/prepareRename";       p=@{ textDocument=$docRef; position=$pos }},
    @{n=28; l="foldingRange";        m="textDocument/foldingRange";        p=@{ textDocument=$docRef }},
    @{n=29; l="selectionRange";      m="textDocument/selectionRange";      p=@{ textDocument=$docRef; positions=@($pos) }},
    @{n=30; l="linkedEditingRange";  m="textDocument/linkedEditingRange";  p=@{ textDocument=$docRef; position=$pos }},
    @{n=31; l="semanticTokens/full"; m="textDocument/semanticTokens/full"; p=@{ textDocument=$docRef }},
    @{n=32; l="semanticTokens/range";m="textDocument/semanticTokens/range";p=@{ textDocument=$docRef; range=@{start=$pos;end=$pos} }},
    @{n=33; l="prepareCallHierarchy";m="textDocument/prepareCallHierarchy";p=@{ textDocument=$docRef; position=$pos }},
    @{n=34; l="callHierarchy/incoming";m="callHierarchy/incomingCalls";   p=@{ item=@{uri=$docUri;name="main";kind=12;ranges=@();selectionRange=@{start=$pos;end=$pos}} }},
    @{n=35; l="callHierarchy/outgoing";m="callHierarchy/outgoingCalls";   p=@{ item=@{uri=$docUri;name="main";kind=12;ranges=@();selectionRange=@{start=$pos;end=$pos}} }},
    @{n=36; l="prepareTypeHierarchy";m="textDocument/prepareTypeHierarchy";p=@{ textDocument=$docRef; position=$pos }},
    @{n=37; l="typeHierarchy/super"; m="typeHierarchy/supertypes";         p=@{ item=@{uri=$docUri;name="main";kind=5;selectionRange=@{start=$pos;end=$pos}} }},
    @{n=38; l="typeHierarchy/sub";   m="typeHierarchy/subtypes";           p=@{ item=@{uri=$docUri;name="main";kind=5;selectionRange=@{start=$pos;end=$pos}} }},
    @{n=39; l="inlayHint";           m="textDocument/inlayHint";           p=@{ textDocument=$docRef; range=@{start=$pos;end=$pos} }},
    @{n=40; l="inlayHint/resolve";   m="inlayHint/resolve";                p=@{ position=$pos; label="hint"; kind=1 }},
    @{n=41; l="inlineValue";         m="textDocument/inlineValue";         p=@{ textDocument=$docRef; range=@{start=$pos;end=$pos} }},
    @{n=42; l="inlineCompletion";    m="textDocument/inlineCompletion";    p=@{ textDocument=$docRef; position=$pos; context=@{triggerKind=1} }},
    @{n=43; l="moniker";             m="textDocument/moniker";             p=@{ textDocument=$docRef; position=$pos }},
    @{n=44; l="diagnostic (pull)";   m="textDocument/diagnostic";          p=@{ textDocument=$docRef }},
    @{n=45; l="completionItem/resolve"; m="completionItem/resolve";        p=@{ label="echo_test"; kind=1 }},
    @{n=46; l="workspace/symbol";    m="workspace/symbol";                 p=@{ query="test" }},
    @{n=47; l="workspace/configuration"; m="workspace/configuration";      p=@{ items=@(@{scopeUri=$docUri;section="editor"}) }},
    @{n=48; l="workspace/executeCommand"; m="workspace/executeCommand";    p=@{ command="test"; arguments=@() }},
    @{n=49; l="workspace/willCreate"; m="workspace/willCreateFiles";       p=@{ files=@(@{uri="file:///new.rave"}) }},
    @{n=50; l="workspace/willRename"; m="workspace/willRenameFiles";       p=@{ files=@(@{oldUri="file:///a";newUri="file:///b"}) }},
    @{n=51; l="workspace/willDelete"; m="workspace/willDeleteFiles";       p=@{ files=@(@{uri="file:///del.rave"}) }},
    @{n=52; l="workspace/applyEdit";  m="workspace/applyEdit";             p=@{ edit=@{}; label="test" }},
    @{n=53; l="window/showMsgReq";    m="window/showMessageRequest";       p=@{ type=1; message="hello"; actions=@(@{title="OK"}) }},
    @{n=54; l="win/progressCreate";   m="window/workDoneProgress/create";  p=@{ token="t1" }},
    @{n=55; l="win/progressCancel";   m="window/workDoneProgress/cancel";  p=@{ token="t1" }},
    @{n=56; l="client/register";      m="client/registerCapability";       p=@{ registrations=@(@{id="r1";method="test"}) }},
    @{n=57; l="client/unregister";    m="client/unregisterCapability";     p=@{ unregisterations=@(@{id="r1";method="test"}) }}
)

$errs = 0
foreach ($t in $tests) {
    Print-Step $t.n $t.l
    Send-Message $sw @{ jsonrpc="2.0"; id=$id; method=$t.m; params=$t.p }
    $rsp = Read-Response $sr
    $obj = try { $rsp | ConvertFrom-Json } catch { $null }
    if ($obj.error) {
        Write-Host "   ← ERROR code=$($obj.error.code) msg=$($obj.error.message)" -ForegroundColor Red
        $errs++
    } else {
        Print-Response $t.l $rsp
    }
    $id++
}

# ═══════════════════════════════════════════════════════════════════════
# Notifications (no response)
# ═══════════════════════════════════════════════════════════════════════

$notifs = @(
    @{n=58; l="didChangeConfiguration";  m="workspace/didChangeConfiguration";  p=@{settings=@{}}},
    @{n=59; l="didChangeWatchedFiles";   m="workspace/didChangeWatchedFiles";   p=@{changes=@()}},
    @{n=60; l="didChangeWorkspaceFolders";m="workspace/didChangeWorkspaceFolders";p=@{event=@{added=@();removed=@()}}},
    @{n=61; l="didCreateFiles";          m="workspace/didCreateFiles";          p=@{files=@()}},
    @{n=62; l="didRenameFiles";          m="workspace/didRenameFiles";          p=@{files=@()}},
    @{n=63; l="didDeleteFiles";          m="workspace/didDeleteFiles";          p=@{files=@()}},
    @{n=64; l="didChange (text)";       m="textDocument/didChange";            p=@{textDocument=@{uri=$docUri;version=2};contentChanges=@(@{text="fn main()->i32{return 0;}"})}},
    @{n=65; l="didSave";                m="textDocument/didSave";               p=@{textDocument=@{uri=$docUri}}},
    @{n=66; l="didClose";               m="textDocument/didClose";              p=@{textDocument=@{uri=$docUri}}},
    @{n=67; l="didRename (text)";       m="textDocument/didRename";             p=@{textDocument=@{uri=$docUri};oldUri=$docUri;newUri="file:///renamed.rave"}},
    @{n=68; l="willSave";               m="textDocument/willSave";              p=@{textDocument=@{uri=$docUri};reason=1}},
    @{n=69; l="$/cancelRequest";        m="$/cancelRequest";                    p=@{id=1}},
    @{n=70; l="telemetry/event";        m="telemetry/event";                    p=@{data="test"}},
    @{n=71; l="$/progress";             m="$/progress";                         p=@{token="t1";value=@{kind="report";message="working"}}},
    @{n=72; l="notebook/didOpen";       m="notebookDocument/didOpen";           p=@{notebookDocument=@{uri="file:///nb.rave"};cellTextDocuments=@()}},
    @{n=73; l="notebook/didChange";     m="notebookDocument/didChange";         p=@{notebookDocument=@{uri="file:///nb.rave"};change=@{}}},
    @{n=74; l="notebook/didSave";       m="notebookDocument/didSave";           p=@{notebookDocument=@{uri="file:///nb.rave"}}},
    @{n=75; l="notebook/didClose";      m="notebookDocument/didClose";          p=@{notebookDocument=@{uri="file:///nb.rave"};cellTextDocuments=@()}}
)

foreach ($n in $notifs) {
    Print-Step $n.n $n.l
    Send-Message $sw @{ jsonrpc="2.0"; method=$n.m; params=$n.p }
    Print-NoResponse
}

# ═══════════════════════════════════════════════════════════════════════
# Shutdown + Exit
# ═══════════════════════════════════════════════════════════════════════
Print-Step 76 "shutdown"
Send-Message $sw @{ jsonrpc="2.0"; id=$id; method="shutdown"; params=@{} }
Print-Response "shutdown" (Read-Response $sr)

Print-Step 77 "exit"
Send-Message $sw @{ jsonrpc="2.0"; method="exit"; params=@{} }
Print-NoResponse

$sw.Close()
Start-Sleep -Milliseconds 300
if (-not $proc.HasExited) { $proc.Kill() }

# ── Summary ──────────────────────────────────────────────────────────
Write-Host "`n=== Complete ===" -ForegroundColor Green
Write-Host "   Requests:    $id" -ForegroundColor White
if ($errs -gt 0) {
    Write-Host "   Errors:      $errs" -ForegroundColor Red
} else {
    Write-Host "   Errors:      $errs" -ForegroundColor Green
}
