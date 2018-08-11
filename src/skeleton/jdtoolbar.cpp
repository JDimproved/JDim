// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdtoolbar.h"

#include "jdlib/miscmsg.h"

#include "config/globalconf.h"

#include <gtk/gtk.h>

#if !GTKMM_CHECK_VERSION(3,0,0)

///////////////////////////////////////////
//
// gtk+-2.12.9/gtk/gtktoolbar.c より引用
//
// gtkのバージョンが上がったときに誤動作しないかどうか注意
// 

typedef struct _GtkToolbarPrivate    GtkToolbarPrivate;
typedef struct _ToolbarContent ToolbarContent;

typedef enum {
  DONT_KNOW,
  OLD_API,
  NEW_API
} ApiMode;

typedef enum {
  TOOL_ITEM,
  COMPATIBILITY
} ContentType;

typedef enum {
  NOT_ALLOCATED,
  NORMAL,
  HIDDEN,
  OVERFLOWN
} ItemState;

struct _GtkToolbarPrivate
{
  GList *       content;
 
  GtkWidget *   arrow;
  GtkWidget *   arrow_button;
  GtkMenu *     menu;
 
  GdkWindow *   event_window;
  ApiMode       api_mode;
  GtkSettings * settings;
  int           idle_id;
  GtkToolItem * highlight_tool_item;
  gint          max_homogeneous_pixels;
 
  GTimer *      timer;

  gulong        settings_connection;

  guint         show_arrow : 1;
  guint         need_sync : 1;
  guint         is_sliding : 1;
  guint         need_rebuild : 1;  /* whether the overflow menu should be regenerated */
  guint         animation : 1;
};

struct _ToolbarContent
{
  ContentType   type;
  ItemState     state;
 
  union
  {
    struct
    {
      GtkToolItem *     item;
      GtkAllocation     start_allocation;
      GtkAllocation     goal_allocation;
      guint             is_placeholder : 1;
      guint             disappearing : 1;
      guint             has_menu : 2;
    } tool_item;
   
    struct
    {
      GtkToolbarChild   child;
      GtkAllocation     space_allocation;
      guint             space_visible : 1;
    } compatibility;
  } u;
};

#define GTK_TOOLBAR_GET_PRIVATE(o)  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_TOOLBAR, GtkToolbarPrivate))

static void
customized_toolbar_content_expose (ToolbarContent *content,
                        GtkContainer   *container,
                        GdkEventExpose *expose)
{
    GtkToolbarChild *child;
    GtkWidget *widget = NULL; /* quiet gcc */
 
    switch (content->type)
    {
        case TOOL_ITEM:
            if (!content->u.tool_item.is_placeholder)
                widget = GTK_WIDGET (content->u.tool_item.item);
            break;

        case COMPATIBILITY:
            child = &(content->u.compatibility.child);

            if (child->type == GTK_TOOLBAR_CHILD_SPACE)
            {
                MISC::ERRMSG( "customized_toolbar_content_expose : CHILD_SPACE is not implemented" );
                return;
            }

            widget = child->widget;
            break;
    }

    if (widget)
        gtk_container_propagate_expose (container, widget, expose);
}

static gint
customized_gtk_toolbar_expose (GtkWidget      *widget,
                               GdkEventExpose *event)
{
#ifdef _DEBUG
    std::cout << "customized_gtk_toolbar_expose\n";
#endif

    GtkToolbar *toolbar = GTK_TOOLBAR (widget);
    GtkToolbarPrivate *priv = GTK_TOOLBAR_GET_PRIVATE (toolbar);
 
    GList *list;

    // 背景描画をキャンセル
/*
    gint border_width;
    border_width = GTK_CONTAINER (widget)->border_width;
    if (GTK_WIDGET_DRAWABLE (widget))
    {
        gtk_paint_box (widget->style,
                       widget->window,
                       (GtkStateType)GTK_WIDGET_STATE (widget),
                       get_shadow_type(toolbar),
                       &event->area, widget, "toolbar",
                       border_width + widget->allocation.x,
                       border_width + widget->allocation.y,
                       widget->allocation.width - 2 * border_width,
                       widget->allocation.height - 2 * border_width
//                       widget->allocation.height + 36
            );
    }
*/

    for (list = priv->content; list != NULL; list = list->next)
    {
        ToolbarContent *content = (ToolbarContent*)list->data;

        customized_toolbar_content_expose (content, GTK_CONTAINER (widget), event);
    }

    gtk_container_propagate_expose (GTK_CONTAINER (widget),
                                    priv->arrow_button,
                                    event);

    return false;
}


///////////////////////////////////////////////////


using namespace SKELETON;


bool JDToolbar::on_expose_event( GdkEventExpose* event )
{
#ifdef _DEBUG
    std::cout << "JDToolbar::on_expose_event\n";
#endif

    if( CONFIG::get_draw_toolbarback() ){

        return Gtk::Toolbar::on_expose_event( event );
    }

    // 自前で描画処理( 背景の描画をしない )
    GtkWidget* widget = dynamic_cast< Gtk::Widget* >( this )->gobj();
    if( widget ) return customized_gtk_toolbar_expose( widget, event );

    return FALSE;
}

#endif // !GTKMM_CHECK_VERSION(3,0,0)
