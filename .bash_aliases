. ~/.profile
# build cpp
build() {
  pip install --verbose --user uplcio_cpp/
}
# install for development
install() {
  build && pip install --user --editable .[test]
}
