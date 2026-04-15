We have to compile program with **-lprofile** param.

```
$ clang++-17 -v -g -O3 -mavx2 -Wall -pedantic program.cpp -lprofiler -o example
```

Then run it, with flags for compiler

```
$ CPUPROFILE=prof.data CPUPROFILE_FREQUENCY=1000 ./example
```

To see into data: 

```
$ google-pprof ./example prof.data

(pprof) text                        
Total: 343 samples
      92  26.8%  26.8%       92  26.8% do_sin
      65  19.0%  45.8%       65  19.0% do_cos
      53  15.5%  61.2%       53  15.5% __sin_fma
      40  11.7%  72.9%       40  11.7% do_sincos
      38  11.1%  84.0%       38  11.1% reduce_sincos
      16   4.7%  88.6%       16   4.7% libc_feholdsetround_sse_ctx
       5   1.5%  90.1%        5   1.5% 0x00005f7c2bcc7040
       4   1.2%  91.3%        4   1.2% 0x00005f7c2bcc7443
       3   0.9%  92.1%        3   0.9% 0x00005f7c2bcc7421
       3   0.9%  93.0%        3   0.9% 0x00005f7c2bcc8eb8

(pprof) text --lines
Total: 343 samples
      31   9.0%   9.0%       31   9.0% do_sin ./math/../sysdeps/ieee754/dbl-64/s_sin.c:144
      28   8.2%  17.2%       28   8.2% __sin_fma ./math/../sysdeps/x86_64/fpu/multiarch/s_sin-fma.c:201
      21   6.1%  23.3%       21   6.1% do_sincos ./math/../sysdeps/ieee754/dbl-64/s_sin.c:116
      16   4.7%  28.0%       16   4.7% do_sin ./math/../sysdeps/ieee754/dbl-64/s_sin.c:142
      16   4.7%  32.7%       16   4.7% libc_feholdsetround_sse_ctx ./math/../sysdeps/x86/fpu/fenv_private.h:416
      15   4.4%  37.0%       15   4.4% do_cos ./math/../sysdeps/ieee754/dbl-64/s_sin.c:114
      15   4.4%  41.4%       15   4.4% do_cos ./math/../sysdeps/ieee754/dbl-64/s_sin.c:116
      12   3.5%  44.9%       12   3.5% do_sincos ./math/../sysdeps/ieee754/dbl-64/s_sin.c:189
      12   3.5%  48.4%       12   3.5% reduce_sincos ./math/../sysdeps/ieee754/dbl-64/s_sin.c:168
      11   3.2%  51.6%       11   3.2% do_sin ./math/../sysdeps/ieee754/dbl-64/s_sin.c:143
```

