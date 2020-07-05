sources = [
  'articleadmin.cpp',
  'articleview.cpp',
  'articleviewbase.cpp',
  'articleviewetc.cpp',
  'articleviewinfo.cpp',
  'articleviewpopup.cpp',
  'articleviewpreview.cpp',
  'articleviewsearch.cpp',
  'drawareabase.cpp',
  'drawareainfo.cpp',
  'drawareamain.cpp',
  'drawareapopup.cpp',
  'embeddedimage.cpp',
  'font.cpp',
  'layouttree.cpp',
  'preference.cpp',
  'toolbar.cpp',
  'toolbarsearch.cpp',
  'toolbarsimple.cpp',
]


article_lib = static_library(
  'article', [sources, config_h],
  dependencies : gtkmm_dep,
  include_directories : include_directories('..'),
)
