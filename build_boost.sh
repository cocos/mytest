if [ $# -eq 0 ]; then
    mode=32
else
    mode=$1
fi
cd ext/boost
chmod +x bootstrap.sh && ./bootstrap.sh
./b2 headers
./b2 address-model=$mode link=static runtime-link=static stage --with-system --with-date_time --with-regex --with-random
cd ../..
