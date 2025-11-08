# origin

Inspired by libraries like stb and quelsolaar I want to make a collection
of C libs I can use in other projects. This is a good way to make

# Generics in C

Matlab has a simple solution for complicated type conversions, everything is a complex matrix.
Similarly in C you can make generic containers by simply storing the object size as a number of bytes.

Ergo the dn_vector has a field item_size and we just memcpy void*'s -> generics in C.

