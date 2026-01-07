# Bloodhound deterministic execution support

bloodhound_sources = files(
  'determinism.c',
  'time.c',
  'random.c',
  'io.c',
  'snapshot.c',
  'hypercall.c',
)

libbloodhound = static_library('bloodhound',
                                sources: bloodhound_sources,
                                include_directories: include_directories('..'),
                                dependencies: [glib],
                                name_suffix: 'fa',
                                build_by_default: false)

bloodhound = declare_dependency(link_whole: libbloodhound)
