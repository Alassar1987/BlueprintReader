# BlueprintReader

A plugin for Unreal Engine that extracts Blueprint structure and graph information into a readable text format (Markdown-style).

Useful for documentation, code review, AI-assisted development, or simply understanding complex Blueprints at a glance.

## 📺 Video Tutorial

[YouTube How it works](https://youtu.be/vbsbJF24uAs?list=PLZuZmQd4mRCburxOj_ALrhRZdZcNWdE5e)

## Features

- **One-click extraction** — Right-click any supported asset in Content Browser → "Show BP as MD"
- **Two-panel output** — Structure tab (variables, components, parameters) and Graph tab (execution flow, data connections)
- **Multiple asset types supported:**
  - Actor Blueprints
  - Actor Component Blueprints
  - Blueprint Interfaces
  - Materials & Material Instances
  - Material Functions & Material Function Instances
  - Enumerations
  - Structures

## Installation

1. Download or clone this repository
2. Copy the `BlueprintReader` folder to your project's `Plugins` directory:
```
   YourProject/
   └── Plugins/
       └── BlueprintReader/
           ├── BlueprintReader.uplugin
           ├── Source/
           └── ...
```
3. Restart Unreal Editor or regenerate project files
4. Enable the plugin in Edit → Plugins → BlueprintReader (if not enabled automatically)

## Usage

1. Open Content Browser
2. Right-click on any supported asset
3. Select **"Show BP as MD"** from the context menu
4. A window will open with two tabs:
   - **Structure** — Variables, components, parameters, class info
   - **Graph** — Execution flow, function calls, data connections

**Tip:** Use `Ctrl + Mouse Wheel` to zoom text in the output window.

## Supported Unreal Engine Versions

- Unreal Engine 5.7+

*(Earlier versions not tested, may work with minor modifications)*

## Known Limitations

- Widget Blueprints — Work in progress
- Other Classes — Work in progress  
- Very complex Blueprints with 500+ nodes may take a moment to process

## Roadmap

- [ ] Widget Blueprint support
- [ ] Export to file (.md, .txt)
- [ ] Partly output from Graph

## Support the Project

If you find this plugin useful, consider supporting its development:

☕ **Buy Me a Coffee:** [Here will be coffee]()

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
