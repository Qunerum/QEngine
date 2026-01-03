using System.Collections.Generic;
using System.IO;
using Avalonia.Media.Imaging;
using QEngine.GUI;

public static class Assets
{
    public static string assetsPath => Path.Combine(Directory.GetCurrentDirectory(), "Assets");
    static Dictionary<(string, int), Sprite> _sprites = new();

    public static Sprite ReadImage(string path, int scale = 1)
    {
        path = Path.Combine(assetsPath, path.Replace("\\", "/"));
        if (_sprites.TryGetValue((path, scale), out Sprite sprite))
            return sprite;
        
        using var stream = File.OpenRead(path);
        var bitmap = new Bitmap(stream);

        sprite = new Sprite(bitmap,  scale);
        _sprites[(path,scale)] = sprite;
        
        return sprite;
    }
}