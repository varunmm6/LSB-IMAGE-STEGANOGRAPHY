# 🔐 LSB Image Steganography

### Hide Secret Messages Inside Images Using C

A C-based Image Steganography project that uses the **Least Significant Bit (LSB)** technique to hide secret text messages inside BMP images and retrieve them without visibly changing the original image.

---

## 📌 Project Overview

Steganography is the technique of hiding secret information inside another file, such as an image, audio, or video.

This project implements image steganography using the LSB technique. It embeds secret text data into the least significant bits of image pixel data.

The project supports both encoding and decoding operations.

## ✨ Features

- 🔒 Hide secret text messages inside BMP images.
- 🔓 Extract hidden messages from stego images.
- 🖼️ Supports BMP image files.
- 📄 Stores the secret file extension and file size.
- ✅ Checks image capacity before encoding.
- 🛠️ Command-line interface for encoding and decoding.
- 💻 Developed using C programming.

## ⚙️ Technologies Used

- C Programming
- File Handling
- Bitwise Operations
- Least Significant Bit (LSB) Technique
- GCC Compiler

## 📂 Project Structure

```text
LSB-IMAGE-STEGANOGRAPHY/
│
├── main.c
├── encode.c
├── encode.h
├── decode.c
├── decode.h
├── common.h
├── types.h
├── beautiful.bmp
├── README.md
└── .gitignore
```

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/varunmm6/LSB-IMAGE-STEGANOGRAPHY.git
```

### 2. Navigate to the Project Directory

```bash
cd LSB-IMAGE-STEGANOGRAPHY
```

### 3. Compile the Project

```bash
gcc -Wall -Wextra -o steganography main.c encode.c decode.c
```

## 📝 How to Use

### 🔐 Encoding – Hide a Secret Message

Create a text file named `secret.txt` containing your secret message.

Run:

```bash
./steganography -e beautiful.bmp secret.txt stego.bmp
```

**Arguments:**
- `-e` – Encoding operation
- `beautiful.bmp` – Original BMP image
- `secret.txt` – Secret text file
- `stego.bmp` – Output image containing the hidden message

### 🔓 Decoding – Extract the Secret Message

Run:

```bash
./steganography -d stego.bmp decoded.txt
```

**Arguments:**
- `-d` – Decoding operation
- `stego.bmp` – Image containing hidden data
- `decoded.txt` – Extracted secret message

## 🔍 How LSB Steganography Works

The Least Significant Bit technique hides information by modifying the least significant bits of image pixel data.

For example:

```text
Original Pixel:  10110100
Secret Bit:              1

Modified Pixel: 10110101
```

By replacing selected least significant bits with secret message bits, information can be embedded into the image.

The decoding process reads those bits to reconstruct the original secret message.

## 🎯 Learning Outcomes

Through this project, I gained practical experience in:

- File handling in C.
- Bitwise operations and bit manipulation.
- Structures and pointers.
- Image data processing.
- Modular programming.
- Encoding and decoding algorithms.
- Command-line argument handling.

## 👨‍💻 Author

**Varun M M**

GitHub: [@varunmm6](https://github.com/varunmm6)

## ⚠️ Disclaimer

This project is developed for educational purposes.

LSB steganography hides information but does not encrypt it. Do not use it as a replacement for encryption when protecting sensitive information.

---

⭐ If you find this project interesting, feel free to explore and star the repository!
