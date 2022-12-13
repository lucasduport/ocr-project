#include "widgetGestion.h"

void widgetCleanup(GtkWidget **to_hide, GtkWidget **to_show)
{
	widgetHider(to_hide);
	widgetDisplayer(to_show);
	return;
}

void widgetDisplayer(GtkWidget **widgets)
{
	for (size_t i = 0; widgets[i] != NULL; i++)
	{
		gtk_widget_show(GTK_WIDGET(widgets[i]));
	}
	return;
}

void changeSensivityWidgets(GtkWidget **widget, int sensitive)
{
	if (sensitive)
	{
		for (int i = 0; widget[i] != NULL; i++)
		{
			gtk_widget_set_sensitive(widget[i], TRUE);
		}
	}
	else
	{
		for (int i = 0; widget[i] != NULL; i++)
		{
			gtk_widget_set_sensitive(widget[i], FALSE);
		}
	}
}

void widgetHider(GtkWidget **widgets)
{
	for (size_t i = 0; widgets[i] != NULL; i++)
		gtk_widget_hide(GTK_WIDGET(widgets[i]));
	return;
}

gboolean waitForHideWarning(gpointer data)
{
	GtkLabel *label = (GtkLabel *)data;
	gtk_widget_hide(GTK_WIDGET(label));
	return FALSE;
}

void displayColoredText(GtkLabel *label, char *message, char *color)
{
	char *markup = g_markup_printf_escaped(
		"<span foreground='%s'><span font_desc='Tlwg Typo Bold "
		"12'>%s</span></span>",
		color, message);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_widget_show(GTK_WIDGET(label));
	g_timeout_add_seconds(2, waitForHideWarning, label);
	free(markup);
	return;
}

/*
void displaySolvingState(GtkWidget *pBar, double percent, GtkLabel *label, char
*message)
{
	char color[10];
	double maxWidth = 250;
	strstr(message, "...") ? strcpy(color, "black") : strcpy(color, "green");
	char *markup = g_markup_printf_escaped("<span foreground='%s'><span
font_desc='Tlwg Typo Bold 13'>%s</span></span>", color, message);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	free(markup);
	gtk_widget_set_size_request(GTK_WIDGET(pBar), (int) (maxWidth * percent),
15); return;
}
*/