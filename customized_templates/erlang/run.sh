#@lang=erlang
#@memcheck=false
export HOME=$PWD

erl -noshell -pa /usr/local/lib/msgpack-erlang  -s homework start -s init stop

