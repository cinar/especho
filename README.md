# ESP Echo

Small tool for testing the network timeouts for the ESP packets.

## Build

### ESP Server

Build the server using the host toolchain.

```bash
make
```

### ESP Client

Build the client using the Android toolchain.

```bash
make CC=/opt/android-ndk-r25c/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android31-clang
```

## License

Copyright (c) 2023 Onur Cinar. All Rights Reserved.

The source code is provided under MIT License.
