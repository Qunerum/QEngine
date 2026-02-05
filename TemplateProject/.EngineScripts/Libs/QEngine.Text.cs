using System;
using System.Collections.Generic;
using System.Linq;

using QEngine.Dev.Renderer;
using QEngine.GUI;

namespace QEngine.Text
{
    public class Font
    {
        public Sprite texture;
        public Dictionary<char, Glyph> glyphs;
        public Vector2Int charSize;
        public float defaultFontSize;

        public void CalcAndSetGlyphs(Vector2Int size, string fontName)
        {
            Console.WriteLine($"Calculating glyphs for '{fontName}'...");
            foreach (var key in glyphs.Keys.ToList())
            {
                Glyph g = glyphs[key];
                Vector2Int pos = g.pos * charSize;
                g.SetMinMax(new(
                    pos.x / (float)size.x,
                    pos.y / (float)size.y,
                    (pos.x + charSize.x) / (float)size.x,
                    (pos.y + charSize.y) / (float)size.y));
                glyphs[key] = g;
                Console.WriteLine($"Glyph '{key}' calculated: position: {pos} , uv: {g.uvMin} {g.uvMax}");
            }
        }
    }

    public struct Glyph
    {
        public Vector2Int pos;
        public Vector2 uvMin;
        public Vector2 uvMax;
        public float thick;

        public Glyph(Vector2Int pos, float thick)
        {
            this.pos = pos;
            this.thick = thick;
        }

        public void SetMinMax(Vector4 uv)
        {
            uvMin = new(uv.x, uv.y);
            uvMax = new(uv.z, uv.w);
        }
    }

    public class Text : Component, IRenderable
    {
        public string text = "Text...";
        public Font font = Assets.GetFont("Default");
        public int fontSize = 16;
        public float space = 8;
        public Color color = new(255);
        public Vector2 preferedSize = new();

        public void Draw()
        {
            if (font == null || font.texture == null || font.texture.pixels == null || font.texture.pixels.Length <= 0) return;
            QRenderer.DrawText(font, space, text, transform.position, fontSize, color.to01());
        }
    }
}
