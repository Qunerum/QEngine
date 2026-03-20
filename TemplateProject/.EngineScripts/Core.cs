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
// ReSharper disable All

/// <summary>
/// Global game state manager. 
/// Handles window properties, resolution settings, and application lifecycle.
/// </summary>
public static class Game
{
    static Sdl2Window? win;
    public static void Init(Sdl2Window window) { if (win == null) win = window; }
    
    /// <summary> The title displayed on the window title bar. </summary>
    public static string title = "QEngine Project";
    
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
    static void updateResolution() { if (win == null) return; win.Width = resolution.x; win.Height = resolution.y; }
    
    /// <summary> Gracefully closes the application and releases window resources. </summary>
    public static void Quit() { Logger.Log("Closing app..."); win?.Close(); }
}

static class Core
{
    static readonly Stopwatch stopwatch = new();
    static double fixedTime;
    static float time = 0;
    static void Main()
    {
        Console.Clear();
        Logger.Log("Starting app...");
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
            
            //Delta time
            float frameTime = (float)stopwatch.Elapsed.TotalSeconds;
            stopwatch.Restart();
            
            Time.deltaTime = frameTime;
            time += frameTime; Time.time = time;
            fixedTime += frameTime;
            while (fixedTime > Time.fixedDeltaTime)
            {
                QEngine.Physics.Physics.Step();
                SceneManager.actualScene?.FixedUpdate();
                fixedTime -= Time.fixedDeltaTime;
            }
            
            QRenderer.Begin();
            Update();
            QRenderer.End();
            Input.data = string.Empty;
        }
        gd.WaitForIdle();
        gd.Dispose();
    }
    static void Update()
    {
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
    public static Vector2 position = new();
    public static float Zoom = 1.0f; // Zmień na 1.0f jako domyślny

    /// <summary> 
    /// WORLD SPACE: Converts world pixels to NDC, accounting for Camera Position and Zoom.
    /// Use this for players, enemies, and map objects.
    /// </summary>
    public static Vector2 WorldToNDC(Vector2 worldPos)
    {
        Vector2 viewPos = (worldPos - position) * Zoom;
        return new Vector2(
            viewPos.x / (Game.resolution.x / 2f), 
            -(viewPos.y / (Game.resolution.y / 2f)) // Minus dla poprawnego Y w Veldrid
        );
    }

    /// <summary> 
    /// SCREEN SPACE: Converts pixels directly to NDC, ignoring Camera.
    /// Use this for UI, HUD, and Menus.
    /// </summary>
    public static Vector2 ScreenToNDC(Vector2 screenPos) => new Vector2(screenPos.x / (Game.resolution.x / 2f), -(screenPos.y / (Game.resolution.y / 2f)));
    
    /// <summary> Scales size for the world (with zoom). </summary>
    public static Vector2 WorldSizeToNDC(Vector2 pixelSize) => new((pixelSize.x * Zoom) / (Game.resolution.x / 2f), (pixelSize.y * Zoom) / (Game.resolution.y / 2f));
    /// <summary> Scales size for UI (constant size). </summary>
    public static Vector2 ScreenSizeToNDC(Vector2 pixelSize) => new(pixelSize.x / (Game.resolution.x / 2f), pixelSize.y / (Game.resolution.y / 2f));
}

/// <summary> 
/// Global time management class. 
/// Provides delta time for frame-dependent logic, fixed delta time for physics, 
/// and tracks total elapsed time since the game started.
/// </summary>
public static class Time
{
    /// <summary> The completion time of the last frame in seconds. </summary>
    public static float deltaTime = 0;
    /// <summary> The total time in seconds since the start of the game. </summary>
    public static float time = 0;
    /// <summary> The fixed interval for physics updates (default: 0.02s = 50Hz). </summary>
    public static float fixedDeltaTime { get; private set; } = 0.02f;
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
            Logger.Log($"Adding scene '{name}'");
            scenes.Add(name, scene);
        }
    }
    
    /// <summary>
    /// Switches the active scene, triggers its <see cref="QEScene.Init"/> method,
    /// and clears previous scene data.
    /// </summary>
    /// <param name="name">The identifier of the scene to load.</param>
    public static void GoToScene(string name) 
    { if (scenes.TryGetValue(name, out var scene)) { actualScene = scene; Logger.Log($"Going to '{name}' scene..."); actualScene.Clear(); actualScene.Init(); } }
}

/// <summary>
/// Static utility class for logging messages to the system console with color support.
/// </summary>
public static class Logger
{
    /// <summary> Logs a standard diagnostic message in gray color. </summary>
    /// <param name="message">The message to log. Any object will be converted to a string.</param>
    public static void Log(object message) => printClr($"[QE LOG]: {message}.\n", ConsoleColor.Gray);

    /// <summary> Logs a warning message in yellow color. Used for signaling unexpected but non-critical issues. </summary>
    /// <param name="message">The warning content.</param>
    public static void Warning(object message) => printClr($"[QE WARNING]: {message}\n", ConsoleColor.Yellow);

    /// <summary> Logs an error message in dark red color. Used for critical failures and exceptions. </summary>
    /// <param name="message">The error details.</param>
    public static void Error(object message) => printClr($"[QE ERROR]: {message}!\n", ConsoleColor.DarkRed);

    static void printClr(string msg, ConsoleColor color = ConsoleColor.White)
    {
        Console.ForegroundColor = color;
        Console.Write(msg);
        Console.ResetColor();
    }
}