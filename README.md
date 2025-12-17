# mycrypt-cli

Terminal program for file encryption/decryption with password hashing.

## Usage

```
mycrypt-cli hash <password>
mycrypt-cli <encrypt|decrypt> <filepath> <password> [output_file]
```

## Build

```
make
```

## Run

```
./build/mycrypt-cli.exe hash mypassword
./build/mycrypt-cli.exe encrypt file.txt mypassword
./build/mycrypt-cli.exe decrypt file.txt.enc mypassword
```

## Test

```
make test
```
