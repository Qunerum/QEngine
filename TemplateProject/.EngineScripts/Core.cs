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

/// <summary>
/// Global game state manager. 
/// Handles window properties, resolution settings, and application lifecycle.
/// </summary>
public static class Game
{
    static Sdl2Window? win;
    public static void Init(Sdl2Window window) { if (win != null) win = window; }
    
    /// <summary> The title displayed on the window title bar. </summary>
    public static string title = "QEngine 0.3.0 Game";
    
    /// <summary> 
    /// The current rendering resolution of the game. 
    /// Use <see cref="SetResolution(Vector2Int)"/> to modify this value.
    /// </summary>
    public static Vector2Int resolution { get; private set; } = new(1280, 720);
    /// <summary> The clear color used to fill the background of the window. </summary>
    public static Color background = new(0);
    /// <summary> Whether the window is in fullscreen mode. </summary>
    public static bool fullscreen = false;
    /// <summary> Whether the window can be manually resized by the user. </summary>
    public static bool resizable = true;
    /// <summary> Whether the window borders and title bar are visible. </summary>
    public static bool borderVisible = true;
    
    /// <summary> Sets a new game resolution using a <see cref="Vector2Int"/>. </summary>
    /// <param name="newResolution">The width and height of the window.</param>
    public static void SetResolution(Vector2Int newResolution) { resolution = newResolution; updateResolution(); }
    
    /// <summary> Sets a new game resolution using individual width and height values. </summary>
    /// <param name="Width">Horizontal pixels.</param>
    /// <param name="Height">Vertical pixels.</param>
    public static void SetResolution(int Width, int Height) { resolution = new(Width, Height); updateResolution(); }
    static void updateResolution() { win.Width = resolution.x; win.Height = resolution.y; }
    
    /// <summary> Gracefully closes the application and releases window resources. </summary>
    public static void Quit() { Console.WriteLine("Closing app..."); win?.Close(); }
}

static class Core
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
        
        GraphicsDeviceOptions gdOptions = new GraphicsDeviceOptions(
            debug: false,
            swapchainDepthFormat: null,
            syncToVerticalBlank: true
        );
        GraphicsDevice gd = VeldridStartup.CreateGraphicsDevice(
            window,
            gdOptions,
            GraphicsBackend.Vulkan
        );
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
        Assets.Summary();
        SceneManager.GoToScene("Main");

        stopwatch.Start();
        lastTime = stopwatch.Elapsed.TotalSeconds;
        window.Resized += () => { Game.SetResolution(window.Width, window.Height); };
        
        while (window.Exists)
        {
            InputSnapshot inSnap = window.PumpEvents();
            foreach (var c in inSnap.KeyCharPresses) Input.data += c;
            
            // Window data
            if (window.Title != Game.title) { window.Title = Game.title; }
            WindowState needState = Game.fullscreen ? WindowState.FullScreen : WindowState.Normal;
            if (window.WindowState != needState) { window.WindowState = needState; }
            if (window.BorderVisible != Game.borderVisible) { window.BorderVisible = Game.borderVisible; }
            if (window.Resizable != Game.resizable) { window.Resizable = Game.resizable; }
            
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
            Input.data = string.Empty;
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

/// <summary> Provides coordinate transformation utilities between pixel space and Normalized Device Coordinates (NDC). </summary>
public static class Camera
{
    /// <summary> Current dimensions of the viewport used for coordinate calculations. </summary>
    public static Vector2 ScreenSize = new(Game.resolution.x, Game.resolution.y);
    
    /// <summary> Converts a world pixel position (e.g., 1280x720) to NDC space (-1.0 to 1.0). </summary>
    /// <param name="worldPixelPos">The position in pixels.</param>
    /// <returns>A <see cref="Vector2"/> representing the position in Vulkan/Veldrid clip space.</returns>
    public static Vector2 PixelToNDC(Vector2 worldPixelPos)
    {
        float x = worldPixelPos.x / (ScreenSize.x / 2f);
        float y = worldPixelPos.y / (ScreenSize.y / 2f);
        return new Vector2(x, -y);
    }
    
    /// <summary> Scales a pixel-based size to its equivalent relative size in NDC space. </summary>
    /// <param name="pixelSize">Width and height in pixels.</param>
    /// <returns>Scale factor relative to the screen size.</returns>
    public static Vector2 SizeToNDC(Vector2 pixelSize)
        => new(pixelSize.x / (ScreenSize.x / 2f), pixelSize.y / (ScreenSize.y / 2f));
}

/// <summary>
/// Static utility class providing time measurements from the engine's core.
/// </summary>
public static class Time
{
    /// <summary> 
    /// The completion time of the last frame in seconds (e.g., 0.016 for 60 FPS).
    /// Use this to make movement and animations frame-rate independent.
    /// </summary>
    public static float deltaTime = 0;
}

/// <summary>
/// Manages the lifecycle of game scenes. 
/// Allows registering scenes and switching between them during runtime.
/// </summary>
public static class SceneManager
{
    static Dictionary<string, QEScene> scenes = new();
    /// <summary> Currently active and rendering scene. </summary>
    public static QEScene? actualScene = null;
    
    /// <summary> Registers a new scene to the engine. </summary>
    /// <param name="name">Unique identifier for the scene.</param>
    /// <param name="scene">The scene instance to add.</param>
    public static void AddScene(string name, QEScene scene)
    {
        scene.name = name;
        if (!scenes.ContainsKey(name))
        {
            Console.WriteLine($"Adding scene '{name}'");
            scenes.Add(name, scene);
        }
    }
    
    /// <summary>
    /// Switches the active scene, triggers its <see cref="QEScene.Init"/> method,
    /// and clears previous scene data.
    /// </summary>
    /// <param name="name">The identifier of the scene to load.</param>
    public static void GoToScene(string name) 
    { if (scenes.TryGetValue(name, out var scene)) { actualScene = scene; Console.WriteLine($"Going to '{name}' scene..."); actualScene.Clear(); actualScene.Init(); } }
}