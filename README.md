# irMagician

赤外線リモコンシステムのirMagicianをコントロールするためのライブラリです。
ir.hppを組み込んで使用します。

## Requirement/必要なもの
開発環境はpuppylinux-tahrpup64
※ubuntuベース(Trusty Tahr)なのでubuntuでも動くと思いますが試してません。

	JSON for Modern C++ https://github.com/nlohmann/json
json.hppを同じフォルダに置いてください。

## Install/インストール

/dev/ttyACM0は時々/dev/ttyACM1になることがある。
そうなったらデバイスがオープンできなくなって処理が中断してしまうので

	/dev/irMagician
udevルールを定義して、新しいデバイスのシンボリックリンクを作成して
デバイスを固定化する。

	/etc/udev/rules.d/99-irMagician.rules
を作成する。（コピー）

## usage/使い方
	
	#include "ir.hpp"
	〜〜〜〜〜〜〜
	IR irm;
	if(inputNumber){
		irm.playnumber(inputNumber);
	}

## Demo/デモ

makeを実行するとCLI版のジュールが作成されます。

	$ ./ir-test -c
赤外線リモコン信号を内部に取得します。

	$ ./ir-test -s -f <output_file>
取得した赤外線リモコン信号データをjsonファイルとして保存します。

	$ ./ir-test -p
赤外線リモコン信号データを再生します。

	$ ./ir-test -p -f <input_file>
赤外線リモコン信号データの保存されてるjsonファイルを取り込んで再生します。

	$ ./ir-test -n xxxx
赤外線リモコン信号データの保存されてる複数のjsonファイルを順番に再生していきます。
これはリモコンのキーに対応したファイルを用意しておき
例えば1.json、2.json、3.json
-.jsonというファイルにチャンネル番号入力キーを当てて

	$ ./ir-test -n -123
でチャンネル123に切り替えるという用途に使います。
※ファイル名は1文字(拡張子を抜いて)のみの対応です。

## Licence/ライセンス

MIT

## Author/作者

sdfgori
