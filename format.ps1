pushd .
cd src && dir -recurse -include *.cpp,*.hpp | %{clang-format -style=file -i $_.FullName}
popd