using System.Collections.Generic;
using Avalonia;
using QEngine;

internal class GameCore
{
    public static void Main(string[] args)
    => BuildAvaloniaApp()
        .StartWithClassicDesktopLifetime(args);
    
    public static AppBuilder BuildAvaloniaApp()
        => AppBuilder.Configure<App>()
            .UsePlatformDetect()
            .LogToTrace();
}

public static class Game
{
    public static string title = "QEngine Game";
    public static Vector2Int size = new(1280, 720);
}
public static class SceneManager
{
    static Dictionary<string, QEScene> scenes = new();
    public static QEScene? actualScene = null;
    
    public static void AddScene(string name, QEScene scene)
    {
        scene.name = name;
        if (!scenes.ContainsKey(name))
            scenes.Add(name, scene);
    }
    public static void GoToScene(string name) { if (scenes.TryGetValue(name, out var scene)) { actualScene = scene; actualScene.Init(); } }
}