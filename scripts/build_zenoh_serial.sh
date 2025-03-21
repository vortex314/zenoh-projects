

#!/bin/bash
cd ../../zenoh
HEIGHT=15
WIDTH=40
CHOICE_HEIGHT=4
BACKTITLE="Build zenoh menu"
TITLE="Zenoh build scope"
MENU="Choose one of the following options:"

OPTIONS=(1 "zenohd z_sub withs serial and tcp "
         2 "all-targets with serial and tcp "
         3 "Like 2 only 3 cores")

CHOICE=$(dialog --clear \
                --backtitle "$BACKTITLE" \
                --title "$TITLE" \
                --menu "$MENU" \
                $HEIGHT $WIDTH $CHOICE_HEIGHT \
                "${OPTIONS[@]}" \
                2>&1 >/dev/tty)

clear
case $CHOICE in
        1)
            cargo build --release  --bin zenohd --example z_sub --features transport_tcp,transport_serial
            ;;
        2)
            cargo build --release  --all-targets --no-default-features --features transport_tcp,transport_serial
            ;;
        3)
            cargo build -j 3 --release --all-targets --no-default-features --features transport_tcp,transport_serial
            ;;
esac
exit

git checkout 1.2.1
cargo build --release  --all-targets --no-default-features --features transport_tcp,transport_serial

cargo build --release  --bin zenohd --example z_sub --features transport_tcp --features transport_serial

248

#!/bin/bash
