# 10101937

## Getting started

#### - Cloning the project

Since this project has other git submoudle, use `--recursive` to clone the reference repo:

    git clone --recursive https://gitlab.syntecclub.com/10101937/10101937.git/

#### - Directory

- Solution: {RepoDir}/Bank
- Output: {RepoDir}/Bank/{platform}/{configuration}

#### - Configuration

Currently `Bank` project has two configuration:

- `Debug`: Output debug string and google test.
- `Release`: Final executable binary.

> To switch between different configuration, toggle between `Debug` and `Release` modes in the Visual Studio Solution Explorer.

## Usuage

Double click `BankServer.exe` to execute server first, and then double click `BankClient.exe` to execute the ATM program.

Press `q` to successfully shutdown `BankServer`.