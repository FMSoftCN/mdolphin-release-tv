# mDolphin TV Release

## Introduction

The TV release of mDolphin, which is a HTML5 browser based on WebKit for MiniGUI.

## Depedencies

  * MiniGUI V3.0.12
  * Cairo on MiniGUI V1.8.6
  * c-ares V1.0.4
  * curl V7.21.0
  * libxml2 V2.7.6
  * libxslt V1.1.26
  * openssl V0.9.8k
  * mDolphin Core V3.0.4

## Usage

1. Run the following command:

    ./autogen.sh
    ./configure
    make
    make install

2. Configure option:

    --enable-incoreres      enable to use incore resource<default=no>
    --with-searchengine     The default search engine
    --with-homeurl          The Home URL (string)

