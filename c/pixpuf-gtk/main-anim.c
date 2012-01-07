#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <errno.h>

static GtkWidget *window = NULL;

void
do_images ()
{
	GtkWidget *image;
	GError *error = NULL;
	char *filename;
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Images");
	
	g_signal_connect (window, "destroy",
		G_CALLBACK (gtk_widget_destroyed), &window);
		
	gtk_container_set_border_width (GTK_CONTAINER (window), 8);
	
	filename = "/home/denis/Робочий стіл/animated.gif";
	image = gtk_image_new_from_file (filename);
	
	gtk_container_add (GTK_CONTAINER (window), image);
	
	gtk_widget_show_all (window);
	
}


int main(int argc, char **argv){
		gtk_set_locale();
		gtk_init(&argc, &argv);
		do_images (NULL);
		gtk_main();
}
