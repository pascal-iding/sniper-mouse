# Sniper Mouse

A lightweight macOS utility to adjust mouse sensitivity on-the-fly using your mouse's side buttons (Button 4 and 5).
Primarily designed for online gaming. 
Compatible with macos.

## Features

- **Precision Mode (Button 4):** Drops sensitivity to `0.1` for ultra-fine control.
- **Tactical Mode (Button 5):** Drops sensitivity to `0.5` for balanced precision.
- **Automatic Reset:** Restores your default speed (`1.5`) instantly when the button is released or the program exits.

## Build

To compile the project, ensure you have the Xcode Command Line Tools installed and run:

```bash
gcc main.c -o sniper-mouse -framework IOKit -framework CoreFoundation -framework ApplicationServices
```

## Usage

Run the compiled binary from your terminal:

```bash
./sniper-mouse
```

### Required Permissions

Since this program monitors mouse events globally, it requires **Accessibility** permissions. 

1. When you first run it, macOS may prompt you.
2. If it fails with an error, go to:
   **System Settings > Privacy & Security > Accessibility**
3. Ensure your **Terminal** app is toggled **ON**.

### Troubleshooting

If your mouse sensitivity is not resetted to default on program exit for whatever reason, simply go to settings app
and set the mouse sensitivity to any value and then back to your desired value.

## License

MIT
