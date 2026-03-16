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

/// <summary>
/// Global manager for handling game resources. 
/// Responsible for loading, storing, and retrieving textures and fonts.
/// </summary>
/// <remarks> Access resources using <see cref="GetSprite"/> and <see cref="GetFont"/>. </remarks>
public static class Assets
{
    /// <summary> Path to the main Assets directory. </summary>
    public static string assetsPath => Path.Combine(Directory.GetCurrentDirectory(), "Assets");
    /// <summary> Path to the Fonts directory. </summary>
    public static string fontsPath => Path.Combine(Directory.GetCurrentDirectory(), "Fonts");
    
    static Dictionary<string, Sprite> _sprites = new();
    static Dictionary<string, Font> _fonts = new();
    
    /// <summary> Retrieves a loaded <see cref="Sprite"/> from the internal cache. </summary>
    /// <param name="name">The name of the sprite (usually the filename without extension).</param>
    /// <returns> The requested <see cref="Sprite"/> object if found; otherwise, <see langword="null"/>. </returns>
    /// <example> <code> Sprite player = Assets.GetSprite("player_idle"); </code> </example>
    public static Sprite GetSprite(string name)
        => _sprites.TryGetValue(name, out Sprite? s) ? s : null;
    
    /// <summary> Retrieves a loaded <see cref="Font"/> from the internal dictionary. </summary>
    /// <param name="font">The name of the font resource.</param>
    /// <returns> The <see cref="Font"/> object containing glyph data, or <see langword="null"/> if not loaded. </returns>
    public static Font? GetFont(string font)
        => _fonts.TryGetValue(font, out Font? f) ? f : null;
    
    static bool inited = false;
    public static void Init()
    {
        if (inited) return;
        inited = true;
        InitalizeFonts();
        foreach (var f in Directory.GetFiles(assetsPath, "*.png", SearchOption.AllDirectories))
            if (Path.GetFileNameWithoutExtension(f) != "Atlas") { InitSprite(f, false); }
    }

    static bool summared = false;
    public static void Summary()
    {
        if (summared) return;
        summared = true;
        Console.WriteLine("< = = = > Initalized Sprites < = = = >");
        foreach (var s in _sprites)
            Console.WriteLine($"> Sprite: {s.Key} with size {s.Value.Width}x{s.Value.Height}");
        Console.WriteLine("< = = = > Initalized Fonts < = = = >");
        foreach (var f in _fonts)
            Console.WriteLine($"> Font: '{f.Key}' with {f.Value.glyphs.Count} glyphs");
    }
    static bool calcFont = false;
    public static void CalculateFonts(Vector2Int size)
    {
        if (calcFont) return;
        calcFont = true;
        foreach (var f in _fonts)
            f.Value.CalcAndSetGlyphs(size, f.Key);
    }
    static bool initSprite = false;
    static void InitSprite(string path, bool fromAssets = true)
    {
        if (initSprite) return;
        initSprite = true;
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

    static bool initFont = false;
    static void InitalizeFonts()
    {
        if (initFont) return;
        initFont = true;
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
}
