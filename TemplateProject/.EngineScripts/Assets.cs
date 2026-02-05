using System.Collections.Generic;
using System.IO;
using System;
using System.Globalization;

using QEngine.GUI;
using QEngine.Text;
using QEngine.Dev.Renderer;

using SixLabors.ImageSharp.PixelFormats;
using SixLabors.ImageSharp;
using Image = SixLabors.ImageSharp.Image;

namespace QEngine;
public static class Assets
{
    public static string assetsPath => Path.Combine(Directory.GetCurrentDirectory(), "Assets");
    static string fontsPath => Path.Combine(Directory.GetCurrentDirectory(), "Fonts");
    static Dictionary<string, Sprite> _sprites = new();
    static Dictionary<string, Font> _fonts = new();
    public static void Init()
    {
        InitalizeFonts();
        foreach (var f in Directory.GetFiles(assetsPath, "*.png", SearchOption.AllDirectories))
            if (Path.GetFileNameWithoutExtension(f) != "Atlas") { InitSprite(f, false); }
    }

    public static void Summary()
    {
        Console.WriteLine("< = = = > Initalized Sprites < = = = >");
        foreach (var s in _sprites)
            Console.WriteLine($"> Sprite: {s.Key} with size {s.Value.Width}x{s.Value.Height}");
        Console.WriteLine("< = = = > Initalized Fonts < = = = >");
        foreach (var f in _fonts)
            Console.WriteLine($"> Font: {f.Key} with {f.Value.glyphs.Count} glyphs");
    }
    public static void CalculateFonts(Vector2Int size)
    {
        foreach (var f in _fonts)
            f.Value.CalcAndSetGlyphs(size, f.Key);
    }
    static void InitSprite(string path, bool fromAssets = true)
    {
        string fpath = fromAssets ? Path.Combine(assetsPath, path) : path;
        using Image<Rgba32> img = Image.Load<Rgba32>(fpath);
        byte[] pixels = new byte[img.Width * img.Height * 4];
        img.CopyPixelDataTo(pixels);
        Sprite s = new Sprite(img.Width, img.Height, pixels);
        _sprites.Add(Path.GetFileNameWithoutExtension(fpath), s);
        Console.WriteLine($"Sprite init '{Path.GetFileNameWithoutExtension(fpath)}'");
        Atlas.AddImage(s);
    }

    static Sprite ReadSprite(string path, bool fromAssets = true)
    {
        string fpath = fromAssets ? Path.Combine(assetsPath, path) : path;
        using Image<Rgba32> img = Image.Load<Rgba32>(fpath);
        byte[] pixels = new byte[img.Width * img.Height * 4];
        img.CopyPixelDataTo(pixels);
        return new Sprite(img.Width, img.Height, pixels);
    }

    public static Sprite GetSprite(string name)
    {
        if (_sprites.TryGetValue(name, out Sprite sprite)) return sprite; 
        return null;
    }
    #region Fonts
    public static Font GetFont(string font)
        => _fonts.ContainsKey(font) ? _fonts[font] : null;
    static void InitalizeFonts()
    {
        foreach (string dir in Directory.GetDirectories(fontsPath))
        {
            string name = Path.GetFileName(dir);
            string qefont = Path.Combine(dir, name + ".qefont");
            string png = Path.Combine(dir, name + ".png");
            if (!File.Exists(png) || !File.Exists(qefont))
                continue;
            Sprite spng = ReadSprite(png, false); Atlas.AddFont(spng);
            Font f = new Font()
            {
                texture = spng,
                glyphs = readFont(qefont, out Vector2Int s, out float dfs),
                charSize = s,
                defaultFontSize = dfs
            };
            Console.WriteLine($"Font '{name}' created with {f.glyphs.Count} glyphs");
            _fonts.Add(name, f);
        }
    }

    static Dictionary<char, Glyph> readFont(string qef, out Vector2Int size, out float dfs)
    {
        string[] lines = File.ReadAllLines(qef);
        Dictionary<char, Glyph> g = new();
        Vector2Int slotSize = new(32, 48);
        float d = 16;
        for (int i = 0; i < lines.Length; i++)
        {
            string line = lines[i];
            if (string.IsNullOrEmpty(line)) continue;
            // Read config.
            if (i == 0)
            {
                string[] config = line.Split(',');
                d = int.Parse(config[1].Trim());
                string[] slot = config[0].Trim().Split('x');
                slotSize = new(int.Parse(slot[0]), int.Parse(slot[1]));
                continue;
            }
            // Read letters
            char Char = line[0];
            string[] data = line.Substring(1).Split(',');
            string[] posA = data[0].Trim().Split('x');
            Vector2Int pos = new(int.Parse(posA[0]), int.Parse(posA[1]));
            float adv = float.Parse(data[1], CultureInfo.InvariantCulture);
            g.Add(Char, new Glyph(pos, adv));
        }
        size = slotSize;
        dfs = d;
        return g;
    }
    #endregion
}