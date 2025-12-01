# Windows 11 Live Captions Controller

A professional Qt/C++ desktop application that interfaces with Windows 11 Live Captions to provide monitoring, control, and caption text capture capabilities. This project demonstrates advanced Windows API integration, UI Automation, and modern Qt development practices.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%2011-blue.svg)
![Qt](https://img.shields.io/badge/Qt-5.15%2B-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-orange.svg)

## 🎯 Features

### Core Functionality
- **Automatic Launch & Detection** - Launches Windows 11 Live Captions via system protocols
- **Real-time Caption Monitoring** - Captures and displays live caption text using UI Automation API
- **Process Management** - Detects running instances and manages Live Captions lifecycle
- **Caption History** - Timestamped log of all captured captions with export capability
- **Direct Settings Access** - Quick access to Windows Live Captions configuration

### Technical Highlights
- **Windows UI Automation API** integration for accessible caption text extraction
- **COM/OLE** programming for Windows system integration
- **Process enumeration** using Toolhelp32 API
- **UWP app launching** via Shell Execute and PowerShell protocols
- **Modern Qt Widgets** with custom styling and responsive design

## 📸 Screenshots

### Main Interface
![Main Window](screenshots/main-window.png)
*Clean, professional interface with real-time caption monitoring*

### Active Monitoring
![Active Monitoring](screenshots/monitoring-active.png)
*Live caption text capture with timestamps*

### Settings Integration
![Settings Access](screenshots/settings-integration.png)
*Direct access to Windows Live Captions settings*

## 🚀 Getting Started

### Prerequisites
- **Windows 11** (Live Captions is Windows 11 exclusive)
- **Qt 5.15+** or **Qt 6.x**
- **MSVC 2019+** or **MinGW 8.1+** compiler
- **Windows SDK 10.0+** (for UI Automation headers)

### Building from Source

1. **Clone the repository**
```bash
git clone https://github.com/yourusername/Qt-LiveCaption.git
cd Qt-LiveCaption
```

2. **Open in Qt Creator**
   - Open `LiveCaption.pro` in Qt Creator
   - Configure your kit (MSVC recommended)
   - Build the project (Ctrl+B)

3. **Or build via command line**
```bash
qmake LiveCaption.pro
nmake  # or mingw32-make for MinGW
```

4. **Run the application**
```bash
./release/LiveCaption.exe
```

## 💻 Usage

1. **Launch the Application**
   - Run `LiveCaption.exe`
   - The app will check if Live Captions is already running

2. **Start Monitoring**
   - Click "Start Monitoring" to launch Live Captions (if not running)
   - The app will automatically connect and begin capturing caption text

3. **View Captured Captions**
   - All captions appear in the main text area with timestamps
   - Use "Clear History" to reset the display

4. **Configure Settings**
   - Click "Open Windows Settings" to access Live Captions configuration
   - Adjust language, position, and appearance in Windows Settings

5. **Stop Monitoring**
   - Click "Stop Monitoring" to pause caption capture
   - Choose whether to close Live Captions or leave it running

## 🏗️ Architecture

### Key Components

**MainWindow** (`mainwindow.h/cpp`)
- Main application controller
- UI Automation integration
- Process management logic
- Caption text extraction and display

**Windows API Integration**
- `IUIAutomation` - UI Automation for caption text reading
- `Toolhelp32` - Process enumeration and detection
- `ShellExecute` - UWP app launching
- `COM/OLE` - Windows component object model

**Qt Components**
- `QMainWindow` - Main application window
- `QTimer` - Periodic caption polling
- `QProcess` - System command execution
- Custom UI styling with Qt Style Sheets

## 🔧 Technical Details

### UI Automation Implementation
```cpp
// Initialize COM and UI Automation
CoInitializeEx(nullptr, COINIT_MULTITHREADED);
CoCreateInstance(__uuidof(CUIAutomation), nullptr, 
                 CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), 
                 (void**)&m_automation);

// Extract caption text from Live Captions window
IUIAutomationElement *rootElement = nullptr;
m_automation->ElementFromHandle(m_captionWindow, &rootElement);
```

### Process Detection
```cpp
// Enumerate processes to detect Live Captions
HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
Process32FirstW(snapshot, &processEntry);
// Check for LiveCaptions.exe in process list
```

### UWP App Launching
```cpp
// Launch via Windows Settings protocol
ShellExecuteW(nullptr, L"open", 
              L"ms-settings:easeofaccess-closedcaptioning", 
              nullptr, nullptr, SW_SHOWNORMAL);
```

## 📋 Project Structure

```
Qt-LiveCaption/
├── main.cpp              # Application entry point
├── mainwindow.h          # Main window header
├── mainwindow.cpp        # Main window implementation
├── mainwindow.ui         # Qt Designer UI file
├── LiveCaption.pro       # Qt project file
├── README.md             # This file
├── LICENSE               # MIT License
└── screenshots/          # Application screenshots
```

## 🎓 Learning Outcomes

This project demonstrates proficiency in:

- **Windows API Programming** - UI Automation, Process Management, Shell Integration
- **COM/OLE Development** - Component Object Model interfaces and lifecycle
- **Qt Framework** - Widgets, signals/slots, event handling, custom styling
- **C++17** - Modern C++ features and best practices
- **Desktop Application Development** - Professional UI/UX design
- **System Integration** - Interfacing with Windows 11 system features

## 🐛 Known Limitations

- **Windows 11 Only** - Live Captions is exclusive to Windows 11
- **UI Automation Dependency** - Caption extraction relies on accessibility APIs
- **UWP Restrictions** - Limited control over Live Captions window properties
- **Language Settings** - Language changes require Windows Settings (no direct API)

## 🔮 Future Enhancements

- [ ] Export captions to text/SRT files
- [ ] Real-time translation integration
- [ ] Custom caption overlay window
- [ ] Speech recognition fallback (Azure/Google APIs)
- [ ] Caption search and filtering
- [ ] Keyboard shortcuts and hotkeys
- [ ] System tray integration
- [ ] Multi-monitor support

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Your Name**
- GitHub: [@James1020-world](https://github.com/James1020-world)

## 🙏 Acknowledgments

- Windows 11 Live Captions team for the accessibility feature
- Qt Project for the excellent cross-platform framework
- Microsoft UI Automation documentation and examples

## 📞 Contact

For questions, suggestions, or collaboration opportunities:
- Email: JamesScott.1020@outlook.com
- GitHub Issues: [Create an issue](https://github.com/James1020-world/Qt-LiveCaption/issues)

---

**Note**: This is a demonstration project showcasing desktop application development skills. It interfaces with Windows 11 Live Captions but does not modify or reverse-engineer the original application.
