# Portager

A simple and visual package manager made for beginners to make Gentoo easy.

## 🚀 Installation

### Option 1: Via Gentoo Overlay (Recommended)
You can easily add my repository to your system:
```bash
sudo eselect repository add finskidomek git https://github.com/finskidomek/portager
sudo emaint sync -r finskidomek
sudo emerge --ask app-portage/portager
```

### Option 2: Manual Git Clone
If you prefer to compile it yourself from source:
```bash
git clone https://github.com/finskidomek/portager
cd portager
make
sudo make install
```

---
☕ *If you find Portager useful and want to support my work, consider [buying me a coffee](https://cuplink.to/koszmar).*
