# project name, version and required libraries 
project('varnish-modules', 'c', version: '"master"')
vapi = dependency('varnishapi')

# find our programs
vtest = find_program('varnishtest')
vmod_tool = find_program(vapi.get_pkgconfig_variable('vmodtool'))
vsc_tool = find_program(vapi.get_pkgconfig_variable('vsctool'))
rst2man = find_program('rst2man')

# we need a config.h file for the version and HAVE_TCP_INFO
conf_data = configuration_data()
conf_data.set('VERSION', meson.project_version())
if meson.get_compiler('c').has_type('struct tcp_info', prefix: '#include <netinet/tcp.h>')
    conf_data.set('HAVE_TCP_INFO', '1')
endif
configure_file( output : 'config.h', configuration : conf_data)

# install the license file
install_data('LICENSE', install_dir: get_option('datadir')+'/doc/varnish-modules')

# list all the test files
vtcs = run_command([ '/bin/sh', '-c', 'cd src/tests && echo */*.vtc']).stdout().strip().split(' ')
foreach vtc: vtcs
    test('command line test', vtest,
        args : [
            '-Dvmod_builddir='+meson.build_root(),
            meson.source_root()+'/src/tests/'+vtc
            ]
        )
endforeach

# list our vmods
vmod_names = [
    'bodyaccess',
    'header',
    'saintmode', 
    'tcp',
    'var',
    'vsthrottle',
    'xkey'
    ]

foreach name: vmod_names
    sources = [ 'src/vmod_'+name+'.c' ]

    vcc_output = custom_target('vcc_'+name+'if.[ch] and RSTs',
        input : 'src/vmod_'+name+'.vcc',
        output : ['vcc_'+name+'_if.c',
            'vcc_'+name+'_if.h',
            'vmod_'+name+'.man.rst',
            'vmod_'+name+'.rst'
            ],
        command : [ vmod_tool,  '@INPUT@', '-o', 'vcc_'+name+'_if'],
        # install only the fourth output file (the .rst)
        install: true,
        install_dir: [false, false, false, get_option('datadir')+'/doc/varnish-modules'])

    sources+= vcc_output

    man_src = [ vcc_output[2] ]
    if name == 'xkey'
        vsc_output = custom_target('VSC_xkey.[ch]',
            input: 'src/xkey.vsc',
            output: [
                'VSC_xkey.c',
                'VSC_xkey.h',
                'VSC_xkey.rst'
                ],
            command : ['/bin/sh', '-c', '$1 -chr $2 > VSC_`basename $2 .vsc`.rst', '/bin/sh', vsc_tool, '@INPUT@'],
            install: true,
            install_dir: [false, false, get_option('datadir')+'/doc/varnish-modules']
            )
        sources+= vsc_output
        man_src+= vsc_output[2]
    endif

    # declare the actual vmod file, override the install directory
    vmod = shared_module('vmod_'+name,
        sources,
        dependencies: vapi,
        install: true,
        install_dir : 'usr/lib/varnish/vmods' 
        )

    # build the man page
    man = custom_target(name+' man page',
        input: man_src[0],
        output: 'vmod_'+name+'.3',
        command : [ rst2man, '@INPUT0@', '@OUTPUT@' ],
        install: true,
        install_dir : get_option('mandir')+'/man3'
        )

    ###############################################################################
    # this is needed only to keep autotools compatibility
    # it can go away if the VTCs use:
    #     import digest from "${vmod_builddir}/libvmod_NAME.so";
    vmod_link = custom_target('vmod_'+name+' link',
        input: vmod,
        output : 'vmod_'+name+'_link',
        command : [
            '/bin/sh', '-c',
            'VMOD='+name+''' &&
            mkdir -p .libs &&
            rm -rf .libs/libvmod_$VMOD.so &&
            ln -s ../libvmod_$VMOD.so .libs/libvmod_$VMOD.so &&
            touch vmod_${VMOD}_link'''
            ],
        build_by_default: true)
endforeach
