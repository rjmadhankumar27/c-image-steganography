# c-image-steganography
A C command-line program that hides text inside BMP images using least significant bit (LSB) encoding and extracts it with a magic string.

## Features

- Encodes a `.txt` file into a `.bmp` image.
- Extracts the hidden text from the generated stego image.
- Uses a magic string to verify that the image contains data encoded by this program.
- Checks whether the image has enough capacity for the message.

## Source files

- `main.c` — command-line entry point and operation selection.
- `encode.c`, `encode.h` — message encoding functions.
- `decode.c`, `decode.h` — message decoding functions.
- `types.h` — shared status and operation types.

## Requirements

- A C compiler such as GCC.
- A 24-bit BMP image to use as the cover image.
- A text file (`.txt`) to hide.

## Build

From the project directory, compile the source files:

```sh
gcc -Wall -Wextra main.c encode.c decode.c -o steganography
```

On Windows with MinGW GCC, the output can be named `steganography.exe`:

```sh
gcc -Wall -Wextra main.c encode.c decode.c -o steganography.exe
```

## Usage

### Encode a text file

```sh
./steganography -e beautiful.bmp secret.txt stego.bmp
```

The program prompts for a magic string. Remember the string; decoding requires the same one. The final output image argument may be omitted, in which case the program uses `stego.bmp`:

```sh
./steganography -e beautiful.bmp secret.txt
```

### Decode a text file

```sh
./steganography -d stego.bmp output.txt
```

The output file argument may be omitted, in which case the program uses `output.txt`:

```sh
./steganography -d stego.bmp
```

The program prompts for the magic string used during encoding. Use a `.txt` output filename because the decoder checks that its extension matches the hidden file extension.

## Notes

- The sample command names assume you have placed the BMP and text files in the project directory. Replace the filenames with your own.
- Only share images and other sample assets that you have permission to publish.
- Do not commit private or personal text. Use generic sample content instead.
- This is an educational project. Review input validation and BMP format handling before relying on it for important data.

## Author

Madhan Kumar R
