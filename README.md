# 🚀 Portager - You are Terminal-free!

I absolutely **love Gentoo Linux**. It is one of the most powerful, flexible, and truly amazing operating systems in the world.

But it hurts me deeply that such an incredible system is often seen as "only for the elite" or "too difficult for normal people." I was a total newbie to Gentoo myself, and I felt that pain. I didn't want the fear of the terminal or complex Portage commands to stop passionate users from experiencing the freedom of Gentoo.

So, I decided to do something about it. I created **Portager**.

Portager is a graphical package manager designed by a newbie, for newbies (and anyone who wants to save some time!). It combines the raw power of Portage with a clean, easy-to-use interface. No more fighting with manual use-flag edits or getting lost in deep dependency trees!

## 💔 The Problem
* Gentoo is amazing, but the learning curve can be brutal.
* New users are often intimidated by the pure CLI package management.
* Resolving use-flags and unmasking packages manually can sometimes feel like a nightmare.

## ❤️ My Mission
To break the barrier. To show the world that anyone can use Gentoo and enjoy building their system from scratch. You don't have to be a hardcore Linux guru anymore.

## 🚀 Installation

### Option 1: Via Gentoo Overlay (Recommended)
You can easily add my repository to your system:

```bash
sudo eselect repository add finskidomek git https://github.com/finskidomek/finskidomek-overlay
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

### ☕ Support the Project
This project is developed with love. If Portager made your Gentoo journey a little easier and you like my work, please consider buying me a coffee:
👉 **[buycoffee.to/koszmar](https://buycoffee.to/koszmar)**

### 🚩 Disclaimer
Please remember that this is a development version! Use it at your own risk. I take no responsibility for any eventual data loss or system damage. Always be careful with your system!
