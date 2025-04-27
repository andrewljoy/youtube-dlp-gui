# youtube-dlp-gui


This is a QT based GUI for the youtube-dlp command-line tool, designed to simplify downloading videos and audio from YouTube, other websites supported by youtube-dlp have not been considered in this code as yet.

**Note:** This code is AI-generated and serves as a starting point for me to learn C/C++  but you are welcome to use, fork, and modify it as you see fit.

## Features

- Input field for YouTube URLs
- Dropdowns for selecting video quality, audio quality, and subtitle languages
- Checkbox to enable SponsorBlock for removing sponsor segments
- Default save path set to Videos, Downloads, or home directory

## Requirements

To build and run this application, you need:

- Qt 5 (or later)
- youtube-dlp installed and accessible in your system's PATH

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/andrewljoy/youtube-dlp-gui.git
   cd youtube-dlp-gui
   ```

2. Install Qt 5 if not already installed. For example, on Fedora:

   ```bash
   sudo dnf install qt5-default qtbase5-dev
   ```

3. Install youtube-dlp:

   ```bash
   pip install yt-dlp
   ```

4. Build the application:

   ```bash
   qmake
   make
   ```

5. Run the application:

   ```bash
   ./youtube_dlp_gui
   ```

## Usage

1. Launch the application.
2. Enter a YouTube URL in the provided field.
3. Select video quality (e.g., 4K, 1080p, or None for audio-only), audio quality (e.g., 320kbps), and subtitle language (e.g., English or None).
4. Check the "Remove sponsor segments" box to use SponsorBlock, if desired.
5. Choose a save folder using the "Choose Folder" button or keep the default.
6. Click "Download" to start the download process.

## Contributing

This project is primarily for learning C/C++, but contributions, suggestions, and improvements are welcome. Feel free to open issues or submit pull requests on GitHub.
