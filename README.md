Converting between numbers and string representation is traditionally done with a simple loop, processing one digits at a time. 
While it's simple and pretty fast, it's inherently inefficient. Many protocols perform this conversion and its performance matters.
It can be made much faster using AVX, and hopefully can be dome with std when C++ gets standard SIMD library.
