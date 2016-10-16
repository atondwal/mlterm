/* -*- c-basic-offset:2; tab-width:2; indent-tabs-mode:nil -*- */

#include <ApplicationServices/ApplicationServices.h>

#include "../ui_font.h"

#include <stdio.h>
#include <string.h> /* memset/strncasecmp */
#include <pobl/bl_debug.h>
#include <pobl/bl_str.h> /* bl_snprintf */
#include <pobl/bl_mem.h> /* alloca */
#include <pobl/bl_str.h> /* bl_str_to_int */
#include <mef/ef_ucs4_map.h>
#include <mef/ef_ucs_property.h>
#include <vt_char_encoding.h> /* ui_convert_to_xft_ucs4 */

#ifdef USE_OT_LAYOUT
#include <otl.h>
#endif

#include "cocoa.h"

#if 0
#define __DEBUG
#endif

/* --- static variables --- */

static int use_point_size;

/* --- static functions --- */

static int parse_font_name(
    char** font_family,
    double* font_size, /* if size is not specified in font_name , not changed. */
    u_int* percent,    /* if percent is not specified in font_name , not changed. */
    char* font_name    /* modified by this function. */
    ) {
  char* p;
  size_t len;

  /*
   * Format.
   * [Family]( [WEIGHT] [SLANT] [SIZE]:[Percentage])
   */

  *font_family = font_name;

  if ((p = strrchr(font_name, ':'))) {
    /* Parsing ":[Percentage]" */
    if (bl_str_to_uint(percent, p + 1)) {
      *p = '\0';
    }
#ifdef DEBUG
    else {
      bl_warn_printf(BL_DEBUG_TAG " Percentage(%s) is illegal.\n", p + 1);
    }
#endif
  }

/*
 * Parsing "[Family] [WEIGHT] [SLANT] [SIZE]".
 * Following is the same as ui_font.c:parse_xft_font_name()
 * except FW_* and is_italic.
 */

#if 0
  bl_debug_printf("Parsing %s for [Family] [Weight] [Slant]\n", *font_family);
#endif

  p = bl_str_chop_spaces(*font_family);
  len = strlen(p);
  while (len > 0) {
    size_t step = 0;

    if (*p == ' ') {
      char* orig_p;

      orig_p = p;
      do {
        p++;
        len--;
      } while (*p == ' ');

      if (len == 0) {
        *orig_p = '\0';

        break;
      } else {
        if (*p != '0' ||      /* In case of "DevLys 010" font family. */
            *(p + 1) == '\0') /* "MS Gothic 0" => "MS Gothic" + "0" */
        {
          char* end;
          double size;

          size = strtod(p, &end);
          if (*end == '\0') {
            /* p has no more parameters. */

            *orig_p = '\0';
            if (size > 0) {
              *font_size = size;
            }

            break;
          }
        }

        step = 1;
      }
    } else {
      step = 1;
    }

    p += step;
    len -= step;
  }

  return 1;
}

/* --- global functions --- */

int ui_compose_dec_special_font(void) {
  /* Do nothing for now in win32. */
  return 0;
}

/* Undocumented */
bool CGFontGetGlyphsForUnichars(CGFontRef, u_int16_t[], CGGlyph[], size_t);

ui_font_t* ui_font_new(Display* display, vt_font_t id, int size_attr, ui_type_engine_t type_engine,
                       ui_font_present_t font_present, const char* fontname, u_int fontsize,
                       u_int col_width, int use_medium_for_bold,
                       u_int letter_space /* Ignored for now. */
                       ) {
  ui_font_t* font;
  char* font_family;
  u_int percent;

  if (type_engine != TYPE_XCORE || (font = calloc(1, sizeof(ui_font_t))) == NULL) {
#ifdef DEBUG
    bl_warn_printf(BL_DEBUG_TAG " malloc() failed.\n");
#endif

    return NULL;
  }

  font->display = display;
  font->id = id;

  if (font->id & FONT_FULLWIDTH) {
    font->cols = 2;
  } else {
    font->cols = 1;
  }

  if (IS_ISCII(FONT_CS(font->id)) || FONT_CS(font->id) == ISO10646_UCS4_1_V) {
    /*
     * For exampe, 'W' width and 'l' width of OR-TTSarala font for ISCII_ORIYA
     * are the same by chance, though it is actually a proportional font.
     */
    font->is_var_col_width = font->is_proportional = 1;
  } else if (font_present & FONT_VAR_WIDTH) {
    font->is_var_col_width = 1;
  }

  if (font_present & FONT_VERTICAL) {
    font->is_vertical = 1;
  }

  font_family = NULL;
  percent = 0;

  if (fontname) {
    char* p;

    if ((p = bl_str_alloca_dup(fontname)) == NULL) {
#ifdef DEBUG
      bl_warn_printf(BL_DEBUG_TAG " alloca() failed.\n");
#endif

      free(font);

      return NULL;
    }

    double fontsize_d = 0;
    parse_font_name(&font_family, &fontsize_d, &percent, p);

    if (fontsize_d > 0.0) {
      fontsize = fontsize_d;
    }
  } else {
    /* Default font */
    font_family = "Menlo";
  }

  char* orig_font_family = font_family;

  if ((font->id & FONT_ITALIC) && !strcasestr(font_family, "italic")) {
    char* p = alloca(strlen(font_family) + 8);

    sprintf(p, "%s Italic", font_family);
    font_family = p;
  }

  if (font->id & FONT_BOLD) {
#if 1
    if (!strcasestr(font_family, "Bold")) {
      char* p = alloca(strlen(font_family) + 6);

      sprintf(p, "%s Bold", font_family);
      font_family = p;
    }
#else
    font->double_draw_gap = 1;
#endif
  }

  while (!(font->cg_font = cocoa_create_font(font_family))) {
    bl_warn_printf("%s font is not found.\n", font_family ? font_family : "Default");

    if (orig_font_family && (font->id & (FONT_ITALIC | FONT_BOLD))) {
      font_family = orig_font_family;

      if (font->id & FONT_BOLD) {
        font->double_draw_gap = 1;
      }

      orig_font_family = NULL;
    } else if (col_width == 0 && strcmp(font_family, "Menlo") != 0) {
      /* standard(usascii) font */

      font_family = "Menlo";
    } else {
      free(font);

      return NULL;
    }
  }

#if 0
  bl_debug_printf("%d %d %d %d %d %d\n", CGFontGetAscent(font->cg_font),
                  CGFontGetDescent(font->cg_font), CGFontGetLeading(font->cg_font),
                  CGFontGetCapHeight(font->cg_font), CGFontGetXHeight(font->cg_font),
                  CGFontGetUnitsPerEm(font->cg_font));
#endif

  font->size = fontsize;

  int ascent = CGFontGetAscent(font->cg_font);
  int descent = CGFontGetDescent(font->cg_font); /* minus value */
  int units = CGFontGetUnitsPerEm(font->cg_font);

  font->height = ((float)fontsize * (ascent - descent)) / ((float)units) + 0.5;
  font->ascent = (font->height * ascent) / (ascent - descent);

  CGGlyph glyphs[1];
  u_int16_t ch = 'M';
  int advance;
  CGFontGetGlyphsForUnichars(font->cg_font, &ch, glyphs, 1);
  CGFontGetGlyphAdvances(font->cg_font, glyphs, 1, &advance);
  fontsize = ((float)fontsize * advance * 2) / ((float)units) + 0.5;

  /*
   * Following processing is same as ui_font.c:set_xfont()
   */

  font->x_off = 0;

  if (col_width == 0) {
    /* standard(usascii) font */

    if (percent > 0) {
      u_int ch_width;

      if (font->is_vertical) {
        /*
         * !! Notice !!
         * The width of full and half character font is the same.
         */
        ch_width = font->height * percent / 100;
      } else {
        ch_width = font->height * percent / 200;
      }

      font->width = ch_width;
    } else if (font->is_vertical) {
      /*
       * !! Notice !!
       * The width of full and half character font is the same.
       */

      font->width = fontsize;
    } else {
      font->width = fontsize * font->cols / 2;
    }

    font->width += letter_space;
  } else {
    /* not a standard(usascii) font */

    if (font->is_vertical) {
      font->width = col_width;
    } else {
      font->width = col_width * font->cols;
    }
  }

  font->decsp_font = NULL;

  if (size_attr) {
    font->width *= 2;
    font->x_off *= 2;

    if (size_attr >= DOUBLE_HEIGHT_TOP) {
      font->height *= 2;
      font->ascent *= 2;
    }
  }

  font->size_attr = size_attr;

  return font;
}

int ui_font_delete(ui_font_t* font) {
#ifdef USE_OT_LAYOUT
  if (font->ot_font) {
    otl_close(font->ot_font);
  }
#endif

  cocoa_release_font(font->cg_font);

  free(font);

  return 1;
}

int ui_change_font_cols(ui_font_t* font, u_int cols /* 0 means default value */
                        ) {
  if (cols == 0) {
    if (font->id & FONT_FULLWIDTH) {
      font->cols = 2;
    } else {
      font->cols = 1;
    }
  } else {
    font->cols = cols;
  }

  return 1;
}

#ifdef USE_OT_LAYOUT
int ui_font_has_ot_layout_table(ui_font_t* font) {
  if (!font->ot_font) {
#ifdef USE_HARFBUZZ
    if (font->ot_font_not_found) {
      return 0;
    }

    font->ot_font = otl_open(font->cg_font, 0);
#else
    char* path;

    if (font->ot_font_not_found || !(path = cocoa_get_font_path(font->cg_font))) {
      return 0;
    }

    font->ot_font = otl_open(path, 0);

    free(path);
#endif

    if (!font->ot_font) {
      font->ot_font_not_found = 1;

      return 0;
    }
  }

  return 1;
}

u_int ui_convert_text_to_glyphs(ui_font_t* font, u_int32_t* shaped, u_int shaped_len,
                                int8_t* offsets, u_int8_t* widths, u_int32_t* cmapped,
                                u_int32_t* src, u_int src_len, const char* script,
                                const char* features) {
  return otl_convert_text_to_glyphs(font->ot_font, shaped, shaped_len, offsets, widths, cmapped,
                                    src, src_len, script, features,
                                    font->size * (font->size_attr >= DOUBLE_WIDTH ? 2 : 1));
}
#endif /* USE_OT_LAYOUT */

u_int ui_calculate_char_width(ui_font_t* font, u_int32_t ch, ef_charset_t cs, int* draw_alone) {
  if (draw_alone) {
    *draw_alone = 0;
  }

  if (font->is_proportional) {
#ifdef USE_OT_LAYOUT
    if (font->use_ot_layout /* && font->ot_font */) {
      return cocoa_font_get_advance(font->cg_font, font->size, font->size_attr, NULL, 0, ch);
    } else
#endif
    {
      u_int16_t utf16[2];
      u_int len;

      if ((len = ui_convert_ucs4_to_utf16(utf16, ch) / 2) > 0) {
        return cocoa_font_get_advance(font->cg_font, font->size, font->size_attr, utf16, len, 0);
      } else {
        return 0;
      }
    }
  }

  return font->width;
}

void ui_font_use_point_size(int use) { use_point_size = use; }

/* Return written size */
size_t ui_convert_ucs4_to_utf16(u_char* dst, /* 4 bytes. Little endian. */
                                u_int32_t src) {
  if (src < 0x10000) {
    dst[1] = (src >> 8) & 0xff;
    dst[0] = src & 0xff;

    return 2;
  } else if (src < 0x110000) {
    /* surrogate pair */

    u_char c;

    src -= 0x10000;
    c = (u_char)(src / (0x100 * 0x400));
    src -= (c * 0x100 * 0x400);
    dst[1] = c + 0xd8;

    c = (u_char)(src / 0x400);
    src -= (c * 0x400);
    dst[0] = c;

    c = (u_char)(src / 0x100);
    src -= (c * 0x100);
    dst[3] = c + 0xdc;
    dst[2] = (u_char)src;

    return 4;
  }

  return 0;
}

#ifdef DEBUG

int ui_font_dump(ui_font_t* font) {
  bl_msg_printf("  id %x: Font %p", font->id, font->cg_font);

  if (font->is_proportional) {
    bl_msg_printf(" (proportional)");
  }
  bl_msg_printf("\n");

  return 1;
}

#endif