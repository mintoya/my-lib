
def main [ammount:int] {
  do {
    mut $filec = "\n"
    $filec = $filec + "#ifndef GENERICS_H\n"
    $filec = $filec + "#define GENERICS_H\n"

    for i in 1..$ammount {
      $filec = $filec + $" #define GENERIC_TYPE_($i)\n"
    }
    $filec = $filec + "#endif //GENERICS_H\n"
    $filec = $filec + "#define GENERIC_ELEMENT(type) type:#type,\n"
    $filec = $filec + "#define GENERIC_NAME(val) \\\n"
    $filec = $filec + "_Generic((val), \\\n"
    for i in 1..$ammount {
      $filec = $filec + $" GENERIC_TYPE_($i) \\\n"
    }
    $filec = $filec + "default:\"\")\n"
    $filec = $filec + "#define EXPAND_(x) x\n"
    $filec = $filec + "#define EXPAND(x) EXPAND_(x)\n"

    $filec = $filec + "#if GENERIC_NAME_N == 0\n"
    $filec = $filec + " #undef GENERIC_NAME_N\n"
    $filec = $filec + " #define GENERIC_NAME_N 1\n"

    for i in 1..$ammount {
      $filec = $filec + $" #elif GENERIC_NAME_N == ($i)\n"
      $filec = $filec + $" #undef GENERIC_NAME_N\n"
      $filec = $filec + $" #define GENERIC_NAME_N ($i + 1)\n"
    }
    $filec = $filec + "#endif\n"

    $filec = $filec + "#if GENERIC_NAME_N == 1\n"
    $filec = $filec + "#define GENERIC_TYPE_1 GENERIC_ELEMENT(argtype1)\n"
    for i in 2..$ammount {
      $filec = $filec + $"#elif GENERIC_NAME_N == ($i)\n"
      $filec = $filec + $"#define GENERIC_TYPE_($i) GENERIC_ELEMENT\(argtype($i)\)\n"
    }
    $filec = $filec + "#endif\n"

    $filec = $filec + "#define GEN_CAT_(a, b) a##b\n"
    $filec = $filec + "#define GEN_CAT(a, b) GEN_CAT_(a, b)\n"
    $filec = $filec + "#define STRINGIFY(x) #x\n"
    $filec = $filec + "#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)\n"

    $filec = $filec + "#define MAKE_NEW_TYPE_HELPER(type, newtype)\\\n"
    $filec = $filec + "typedef type newtype;\\\n"
    $filec = $filec + "REGISTER_ALIASED_PRINTER(type, newtype);\n"
    $filec = $filec + "#define MAKE_NEW_TYPE(type)\\\n"
    $filec = $filec + "MAKE_NEW_TYPE_HELPER(type, GEN_CAT(argtype, GENERIC_NAME_N))\n"

    return $filec
  }|save -f genericName.h
}
