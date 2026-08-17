# BlueprintReader

A plugin for Unreal Engine that extracts Blueprint structure and graph information into a readable text format (Markdown-style) — for humans **and** AI agents.

BlueprintReader is the standard access layer for AI agents to UE assets: read via a Slate window, export to `.md` files, or call it natively through **Unreal MCP (UE 5.8)**. One right-click, one MCP tool call — same engine-grade extraction.

## 📺 Video Tutorial

[YouTube How it works](https://youtu.be/vbsbJF24uAs?list=PLZuZmQd4mRCburxOj_ALrhRZdZcNWdE5e)

## Features

- **One-click extraction** — Right-click any supported asset in Content Browser → "Read Blueprint for AI Assistant";
- **Two-panel output** — Structure tab (variables, components, parameters) and Graph tab (execution flow, data connections);
- **File export** — `.md` / `.txt` via the "Export" button;
- **Multiple asset types supported**;
- *(Roadmap)* **Native MCP tools** — read / export / validate assets from any MCP agent (Claude Code, Cursor, Codex, Gemini) over UE 5.8 `ModelContextProtocol`.

## Installation

1. Download from FAB
2. Extract to `YourProject/Plugins/BlueprintReader/`
3. Restart Unreal Editor
4. Enable plugin in Edit → Plugins → BlueprintReader

## How to Use

1. Right-click any supported asset in Content Browser
2. Select "Read Blueprint for AI"
3. View results in Structure/Graph tabs
4. Copy text (Ctrl+A, Ctrl+C) and paste to AI assistant — or use the "Export" button to save a `.md` file.

## Supported Assets

- Actor/Pawn/Character Blueprints;
- Actor Component Blueprints;
- Blueprint Interfaces;
- Materials & Material Instances;
- Material Functions & Material Function Instances;
- Enumerations;
- Structures;
- Widget Blueprints.

## Supported Unreal Engine Versions

- Unreal Engine 5.7
- Unreal Engine 5.8

Single source tree. The plugin builds and works on both.

**Note on EngineVersion:** The `"EngineVersion"` field is intentionally omitted in the source `.uplugin`. This prevents "plugin for older engine" warnings when the source is used in either 5.7 or 5.8 projects. When preparing packages for Fab, set the correct value (`"5.7.0"` or `"5.8.0"`) before running `RunUAT BuildPlugin` for that engine.

## MCP integration (UE 5.8)

BlueprintReader is being extended with a native **MCP toolset** (`UBlueprintReaderToolset`) so any MCP-compatible AI agent can read, export, and validate Blueprints directly:

```
SearchAssets("BP_Enemy")  →  ReadAsset("/Game/Enemies/BP_Enemy")  →  ValidateAsset(...)  →  GetReferences(...)
```

## Roadmap

- **M4 ✅** — Export to file + plugin settings (done)
- **M7** — MCP toolset (Search / Read / Export)
- **M8** — Validation (compile status, broken refs, checksum)
- **M9** — Agent optimization (structured JSON, section-scoped reads)

## Known Limitations

- Other Classes — Work in progress
- Very complex Blueprints with 500+ nodes may take a moment to process

## Support the Project

If you find this plugin useful, consider supporting its development:

🎁 **Patreon:** [Racoon Coder](https://www.patreon.com/c/u12165995)

🎁 **Boosty:** [My Boosty]()

Your support helps me dedicate more time to developing free tools for the Unreal community!

## License

[MIT License](LICENSE) — Free for personal and commercial use.

## Contact & Feedback

- **GitHub Issues:** [Report bugs or request features]()
- **Discord:** [Maybe one day...]()
- **Twitter/X:** [Maybe one day...]()

---

Made with ❤️ by Racoon Coder
