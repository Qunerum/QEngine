using System;
using System.Collections.Generic;
using System.Diagnostics;

using QEngine;
using QEngine.Dev.Renderer;
using QEngine.GUI;
using QEngine.Input;

using Veldrid;
using Veldrid.StartupUtilities;
using Veldrid.Sdl2;

public static class Game
{
    static Sdl2Window win;
    public static void Init(Sdl2Window window) { win = window; }

    public static string title = "QEngine 0.3.0 Game";
    public static Vector2Int resolution = new(1280, 720);
    public static Color background = new(0);
    public static bool fullscreen = false;

    public static void Quit()
    {
        Console.WriteLine("Closing app...");
        win.Close();
    }
}
internal class Core
{
    static Stopwatch stopwatch = new();
    static double lastTime;

    static void Main()
    {
        Console.Clear();
        Console.WriteLine("Starting app...");
        // Add Main Scene to SceneManager
        SceneManager.AddScene("Main", new MainScene());
        
        var windowCI = new WindowCreateInfo(100, 100,
            Game.resolution.x, Game.resolution.y,
            WindowState.Normal, Game.title);
        Sdl2Window window = VeldridStartup.CreateWindow(ref windowCI);
        GraphicsDevice gd = VeldridStartup.CreateGraphicsDevice(window, new GraphicsDeviceOptions(), GraphicsBackend.Vulkan);
        // Init engine functions
        Game.Init(window);
        Assets.Init();
        QRenderer.Init(gd);
        Atlas.Init(out Vector2Int s);
        Atlas.CreateAtlasTexture(gd);
        QRenderer.CreateAtlasSampler(gd);
        QRenderer.CreateAtlasResourceSet(gd);
        Assets.CalculateFonts(s);
        Input.Init(window);
        SceneManager.GoToScene("Main");

        stopwatch.Start();
        lastTime = stopwatch.Elapsed.TotalSeconds;
        window.Resized += () => { Game.resolution = new(window.Width, window.Height); };
        
        while (window.Exists)
        {
            window.PumpEvents();
            // Window data
            if (window.Width != Game.resolution.x || window.Height != Game.resolution.y) { window.Width = Game.resolution.x; window.Height = Game.resolution.y; }
            if (window.Title != Game.title) { window.Title = Game.title; }
            if (window.CursorVisible != Cursor.isVisible) { window.CursorVisible = Cursor.isVisible; }
            //Camera
            Camera.ScreenSize = new Vector2(window.Width, window.Height);
            //Delta time
            double current = stopwatch.Elapsed.TotalSeconds;
            float dt = (float)(current - lastTime);
            lastTime = current;
            
            QRenderer.Begin();
            Update(dt);
            QRenderer.End();
        }
    }
    static void Update(float deltaTime)
    {
        Time.deltaTime = deltaTime;
        if (SceneManager.actualScene != null)
        {
            SceneManager.actualScene.Update();
            foreach (var obj in SceneManager.actualScene.GetObjects()) 
            { foreach (var com in obj.GetComponents()) { if (com is IRenderable renderable) { renderable.Draw(); } } }
        }
    }
}
public static class Camera
{
    public static Vector2 ScreenSize = new(Game.resolution.x, Game.resolution.y);

    public static Vector2 PixelToNDC(Vector2 worldPixelPos)
    {
        float x = worldPixelPos.x / (ScreenSize.x / 2f);
        float y = worldPixelPos.y / (ScreenSize.y / 2f);
        return new Vector2(x, -y);
    }
    public static Vector2 SizeToNDC(Vector2 pixelSize)
        => new(pixelSize.x / (ScreenSize.x / 2f), pixelSize.y / (ScreenSize.y / 2f));
}

public static class Time { public static float deltaTime = 0; }

public static class SceneManager
{
    static Dictionary<string, QEScene> scenes = new();
    public static QEScene? actualScene = null;
    
    public static void AddScene(string name, QEScene scene)
    {
        scene.name = name;
        if (!scenes.ContainsKey(name))
        {
            Console.WriteLine($"Adding scene '{name}'");
            scenes.Add(name, scene);
        }
    }
    public static void GoToScene(string name) 
    { if (scenes.TryGetValue(name, out var scene)) { actualScene = scene; Console.WriteLine($"Going to '{name}' scene..."); actualScene.Init(); } }
}