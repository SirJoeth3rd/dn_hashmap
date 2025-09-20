# origin

I was working on another project and had to write a hashmap. I liked robin hood hashing so much
I wanted to make generic version of it. Furthermore I also want to write better C code so I decided to
make a generic version of the hashmap.

# generics in C

Generics in C means accepting everything to be passed to the hashmap as a bytes array.
The current implementation is extremely messy because I didn't really do this carefully.
I think this could be made a lot cleaner if I write some wrapper functions to handle the
HashMeta + char[size of item] data.

An idea I just had while writing this is that I could seperate the buffer into two arrays
1. void* item_buffer
2. HashMeta* key_buffer

Include a pointer into the item_buffer as another field in the HashMeta struct, then
simply append values to the void* buffer as new items come in and do key lookup
in the HashMeta* buffer.

What could further increase the interestingness is if the user could supply their own
buffer and allocator as void* item_buffer handler and then for ease of use the HashMap
can just use a default if the user passes these values as NULL.

Then the user could manage the item_buffer themselves and just pass the buffer to the hashmap
and have that hashes into that, very nice. 

