gdk_pixbuf_csource = find_program('gdk-pixbuf-csource')
gen = generator(gdk_pixbuf_csource,
                output : '@BASENAME@.h',
                capture : true,
                arguments : ['--raw', '--build-list', 'icon_@BASENAME@', '@INPUT@'])

icon_files = [
  'bkmark.png',
  'bkmark_broken_subject.png',
  'bkmark_thread.png',
  'bkmark_update.png',
  'board.png',
  'board_update.png',
  'board_updated.png',
  'broken_subject.png',
  'check.png',
  'dir.png',
  'down.png',
  'favorite.png',
  'hist.png',
  'hist_board.png',
  'hist_close.png',
  'hist_closeboard.png',
  'hist_closeimg.png',
  'image.png',
  'info.png',
  'jd16.png',
  'jd32.png',
  'jd48.png',
  'jd96.png',
  'link.png',
  'loading.png',
  'loading_stop.png',
  'newthread.png',
  'newthread_hour.png',
  'post.png',
  'post_refer.png',
  'thread.png',
  'thread_old.png',
  'thread_update.png',
  'thread_updated.png',
  'update.png',
  'write.png',
]

generated_headers = []
foreach f : icon_files
  generated_headers += gen.process(f)
endforeach

sources = [
  'iconmanager.cpp',
]


icon_lib = static_library(
  'icon', [sources, generated_headers],
  dependencies : gtkmm_dep,
  include_directories : include_directories('..'),
)
