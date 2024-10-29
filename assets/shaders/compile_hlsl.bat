::-D <value>              Define macro.
::-E <value>              Entry point name.
::-Fd <file>              Write debug information to the specified file or directory; trail \ to autogenerate.
::-Fo <file>              Output object file.
::-Gec                    Enable backward compatibility mode.
::-HV <value>             HLSL version (2016, 2017, 2018, 2021). Default is 2018.
::-Od                     Disable optimizations.
::-T <profile>            Set target profile.
::    <profile>: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, ps_6_5, ps_6_6,
::                vs_6_0, vs_6_1, vs_6_2, vs_6_3, vs_6_4, vs_6_5, vs_6_6,
::                cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4, cs_6_5, cs_6_6,
::                gs_6_0, gs_6_1, gs_6_2, gs_6_3, gs_6_4, gs_6_5, gs_6_6,
::                ds_6_0, ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, ds_6_6,
::                hs_6_0, hs_6_1, hs_6_2, hs_6_3, hs_6_4, hs_6_5, hs_6_6,
::                lib_6_3, lib_6_4, lib_6_5, lib_6_6,
::-Vd                     Disable validation.
::-Zi                     Enable debug information.
::-Zpc                    Pack matrices in column-major order.
::-Zpr                    Pack matrices in row-major order. 

::\path\to\dxc.exe -spirv -T <target-prfile> -E <entry-point> <hlsl-src-file> -Fo <spirv-bin-file>

dxc.exe -spirv -T vs_6_3 vert.hlsl -Fo vert.spv
dxc.exe -spirv -T ps_6_3 frag.hlsl -Fo frag.spv

::dxc.exe -dumpbin vs.dxil
::dxc.exe -dumpbin ps.dxil

pause