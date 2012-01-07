//gcc `pkg-config --cflags gtk+-3.0` main-prog-anim.c `pkg-config --libs gtk+-3.0` -o main-prog-anim
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <errno.h>

static GtkWidget *window = NULL;
static GdkPixbufLoader *pixbuf_loader = NULL;
static GdkPixbufAnimation * anim = NULL;
static GdkPixbufAnimationIter * iter = NULL;
static guint load_timeout = 0;
static guint update_image_loop = 0;
static FILE* image_stream = NULL;
static guint done = 0;

static gboolean update_image(gpointer data)
{
	g_print("update_image\n");
	if (gdk_pixbuf_animation_iter_advance (iter, NULL))
	{
		GdkPixbuf *pixbuf;
		GtkWidget *image;
		printf("+\n");
		image = GTK_WIDGET (data);  
		pixbuf = gdk_pixbuf_animation_iter_get_pixbuf (iter);
		/* Avoid displaying random memory contents, since the pixbuf
		 * isn't filled in yet.
		 */
		gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
	}	
	
	return TRUE;
}

static void
progressive_prepared_callback (GdkPixbufLoader *loader,
			       gpointer		data)
{
  g_print("progressive_prepared_callback\n");
  if (!anim) {
		gint delay;
		GdkPixbuf *pixbuf;
		GtkWidget *image;

		image = GTK_WIDGET (data);

		anim = gdk_pixbuf_loader_get_animation(loader);
		iter = gdk_pixbuf_animation_get_iter (anim, NULL);   
		pixbuf = gdk_pixbuf_animation_iter_get_pixbuf (iter);
		/* Avoid displaying random memory contents, since the pixbuf
		 * isn't filled in yet.
		 */
		gdk_pixbuf_fill (pixbuf, 0xaaaaaaff);
		gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
		delay = gdk_pixbuf_animation_iter_get_delay_time (iter);
		update_image_loop = g_timeout_add (delay,
						       (GSourceFunc) update_image,
						       data);
	}
}

static void
progressive_updated_callback (GdkPixbufLoader *loader,
                              gint		   x,
                              gint		   y,
                              gint		   width,
                              gint		   height,
                              gpointer	   data)
{
  g_print("progressive_updated_callback\n");
  GtkWidget *image;

  image = GTK_WIDGET (data);

  /* We know the pixbuf inside the GtkImage has changed, but the image
   * itself doesn't know this; so queue a redraw.  If we wanted to be
   * really efficient, we could use a drawing area or something
   * instead of a GtkImage, so we could control the exact position of
   * the pixbuf on the display, then we could queue a draw for only
   * the updated area of the image.
   */

  gtk_widget_queue_draw (image);
}

static gint
progressive_timeout (gpointer data)
{
	g_print("progressive_timeout\n");
	GtkWidget *image;
	
	image = GTK_WIDGET (data);
	
	/* This shows off fully-paranoid error handling, so looks scary.
	* You could factor out the error handling code into a nice separate
	* function to make things nicer.
	*/
	
	if (image_stream)
	{
		size_t bytes_read;
		guchar *buf = g_new(guchar, 8192);
		GError *error = NULL;
		
		bytes_read = fread (buf, 1, 8192, image_stream);
		
		if (ferror (image_stream))
		{
			g_printf (
				"Failure reading image file 'alphatest.png': %s",
				g_strerror (errno)
			);
						
			fclose (image_stream);
			image_stream = NULL;
			
			load_timeout = 0;
			
			return FALSE; /* uninstall the timeout */
		}
	
		if (!gdk_pixbuf_loader_write (pixbuf_loader,
				buf, bytes_read,
			&error))
		{
			g_printf (
			"Failed to load image: %s",
			error->message);
			
			g_error_free (error);		
			
			fclose (image_stream);
			image_stream = NULL;
			load_timeout = 0;
			
			return FALSE; /* uninstall the timeout */
		}
		g_printf("+%ld\n",bytes_read);
		g_free(buf);
		g_printf(".$\n",bytes_read);
		if (feof (image_stream))
		{
			g_printf("+file close+\n",bytes_read);
			fclose (image_stream);
			image_stream = NULL;
			g_printf("+file close start+\n",bytes_read);
			/* Errors can happen on close, e.g. if the image
			* file was truncated we'll know on close that
			* it was incomplete.
			*/
			error = NULL;
			if (!gdk_pixbuf_loader_close (pixbuf_loader,
				&error))
			{
				g_printf (
				"Failed to load image: %s",
				error->message);
				
				g_error_free (error);
				
				g_object_unref (pixbuf_loader);
				pixbuf_loader = NULL;
				
				load_timeout = 0;
				
				return FALSE; /* uninstall the timeout */
			}
			g_printf("+file close done+\n",bytes_read);
			g_object_unref (pixbuf_loader);
			pixbuf_loader = NULL;
			done = 1;
		}
		g_printf(".***)\n",bytes_read);
	}
	else
	{
		gchar *filename;
		gchar *error_message = NULL;
		GError *error = NULL;
		
		filename = "./tc3.gif";
		
		if (error)
		{
			error_message = g_strdup (error->message);
			g_error_free (error);
		}
		else
		{
			image_stream = g_fopen (filename, "rb");
			
			if (!image_stream)
			error_message = g_strdup_printf ("Unable to open image file 'alphatest.png': %s",
			g_strerror (errno));
		}
		
		if (image_stream == NULL)
		{
			g_printf (
			"%s", error_message);
			g_free (error_message);
						
			load_timeout = 0;
			
			return FALSE; /* uninstall the timeout */
		}
	
		if (pixbuf_loader)
		{
			gdk_pixbuf_loader_close (pixbuf_loader, NULL);
			g_object_unref (pixbuf_loader);
			pixbuf_loader = NULL;
		}
	
		pixbuf_loader = gdk_pixbuf_loader_new ();
	
		g_signal_connect (pixbuf_loader, "area-prepared",
			G_CALLBACK (progressive_prepared_callback), image);
	
		g_signal_connect (pixbuf_loader, "area-updated",
			G_CALLBACK (progressive_updated_callback), image);
	}
	g_printf(".---\n");
	/* leave timeout installed */
	if (!done)
		return TRUE;
	else
		return FALSE;
}

static void
start_progressive_loading (GtkWidget *image)
{
  /* This is obviously totally contrived (we slow down loading
   * on purpose to show how incremental loading works).
   * The real purpose of incremental loading is the case where
   * you are reading data from a slow source such as the network.
   * The timeout simply simulates a slow data source by inserting
   * pauses in the reading process.
   */
  load_timeout = gdk_threads_add_timeout (150,
				progressive_timeout,
				image);
}

static void
cleanup_callback (GObject *object,
		  gpointer   data)
{
  if (load_timeout)
    {
      g_source_remove (load_timeout);
      load_timeout = 0;
    }

  if (update_image_loop)
    {
      g_source_remove (update_image_loop);
      update_image_loop = 0;
    }

  if (pixbuf_loader)
    {
      gdk_pixbuf_loader_close (pixbuf_loader, NULL);
      g_object_unref (pixbuf_loader);
      pixbuf_loader = NULL;
    }
	
  if(anim)
	g_object_unref(anim);
		
  if (image_stream)
    fclose (image_stream);
  image_stream = NULL;
  
  
  gtk_main_quit ();
}

void
do_images ()
{
	GtkWidget *image;
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Images");
	
	g_signal_connect (window, "destroy",
	G_CALLBACK (gtk_widget_destroyed), &window);
	g_signal_connect (window, "destroy",
	G_CALLBACK (cleanup_callback), NULL);
	
	gtk_container_set_border_width (GTK_CONTAINER (window), 8);
	
	/* Progressive */
	/* Create an empty image for now; the progressive loader
	* will create the pixbuf and fill it in.
	*/
	image = gtk_image_new_from_pixbuf (NULL);
	gtk_container_add (GTK_CONTAINER (window), image);
	
	start_progressive_loading (image);
	
	gtk_widget_show_all (window);
}


int main(int argc, char **argv){
		gtk_init(&argc, &argv);
		do_images (NULL);
		gtk_main();
		return 0;
}
