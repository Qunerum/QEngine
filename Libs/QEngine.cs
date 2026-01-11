using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Avalonia;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Media.Imaging;

using QEngine.GUI;
using QEngine.Mathematics;
using Key = QEngine.Input.Key;

namespace QEngine
{
    #region Variables
    public class Vector2
    {
        public float x, y;
        public Vector2(float x = 0, float y = 0) { this.x = x; this.y = y; }

        public bool isZero => x == 0 && y == 0;
        
        public override string ToString() => $"({x}, {y})";
        
        public static Vector2 operator +(Vector2 a, Vector2 b)
            => new(a.x + b.x, a.y + b.y);
        
        public static Vector2 operator -(Vector2 a, Vector2 b)
            => new(a.x - b.x, a.y - b.y);
        
        public static Vector2 operator *(Vector2 a, Vector2 b)
            => new(a.x * b.x, a.y * b.y);
        public static Vector2 operator *(Vector2 a, float b)
            => new(a.x * b, a.y * b);
        
        public static Vector2 operator /(Vector2 a, Vector2 b)
            => new(a.x / b.x, a.y / b.y);
        public static Vector2 operator /(Vector2 a, float b)
            => new(a.x / b, a.y / b);
    }
    public class Vector2Int
    {
        public int x, y;
        public Vector2Int(int x = 0, int y = 0) { this.x = x; this.y = y; }

        public override string ToString() => $"({x}, {y})";

        public static Vector2Int operator +(Vector2Int a, Vector2Int b)
            => new(a.x + b.x, a.y + b.y);
        
        public static Vector2Int operator -(Vector2Int a, Vector2Int b)
            => new(a.x - b.x, a.y - b.y);
        
        public static Vector2Int operator *(Vector2Int a, Vector2Int b)
            => new(a.x * b.x, a.y * b.y);
        public static Vector2Int operator *(Vector2Int a, int b)
            => new(a.x * b, a.y * b);
    }
    public class Transform
    {
        public Vector2 position = new();
        //public float rotation = 0;
        public Vector2 scale = new(1, 1);
    }
    #endregion
    
    #region Global
    public class QEScene
    {
        public string name;
        readonly List<GameObject> _objects = new();

        public Vector2 cameraPosition = new();
        public void AddGameObject(GameObject obj) => _objects.Add(obj);
        public List<GameObject> GetObjects() => _objects;
        
        public GameObject? GetGameObject(string name) =>_objects.FirstOrDefault(o => o.name == name);
        
        public virtual void Init() {}
        public void Update() { foreach (var obj in _objects) obj.Update(); }
    }
    public sealed class GameObject
    {
        public string name = "";
        public Transform transform = new();
        readonly List<Component> _components = new();
        
        public GameObject(string name = "") { this.name = name; SceneManager.actualScene.AddGameObject(this); }
        
        public T AddComponent<T>() where T : Component, new()
        {
            var c = new T { gameObject = this, transform = transform };
            if (!_components.Contains(c))
            {
                _components.Add(c);
                c.Init();
                return c;
            } else { return GetComponent<T>(); }
        }

        public T? GetComponent<T>() where T : Component
            => _components.OfType<T>().FirstOrDefault();

        public bool TryGetComponent<T>(out T component) where T : Component
        {
            foreach (var c in _components) { if (c is T t) { component = t; return true; } }
            component = null!;
            return false;
        }
        
        internal void Update() { foreach (var c in _components) c.Update(); }
    }
    public class Component
    {
        public GameObject gameObject { get; internal set; } = null;
        public Transform transform { get; internal set; } = null;
        public virtual void Init() {}
        public virtual void Update() {}
    }
    public class QEScript : Component { public QEScene? scene => SceneManager.actualScene; }
    #endregion
}

namespace QEngine.GUI
{
    public static class GUI
    {
        public static bool isOnUI(Vector2 center, Vector2 size, Vector2 scale)
        {
            Vector2Int mp = Input.Input.mousePosition;
            float halfW = size.x * scale.x / 2;
            float halfH = size.y * scale.y / 2;
            return
                mp.x >= center.x - halfW &&
                mp.x <= center.x + halfW &&
                mp.y >= center.y - halfH &&
                mp.y <= center.y + halfH;
        }
    }

    #region Variables
    public class Color
    {
        public byte r, g, b, a;
        public SolidColorBrush _clr;

        public Color(byte rgb = 255, byte a = 255)
        {
            r = rgb; g = rgb; b = rgb; this.a = a; 
            _clr = new(new Avalonia.Media.Color(a, rgb, rgb, rgb));
        }
        public Color(byte r, byte g, byte b, byte a = 255)
        {
            this.r = r; this.g = g; this.b = b; this.a = a;
            _clr = new(new Avalonia.Media.Color(a, r, g, b));
        }
        public static Color Red => new(255, 0, 0);
        public static Color Green => new(0, 255, 0);
        public static Color Blue => new(0, 0, 255);
    }

    public sealed class Sprite
    {
        public Bitmap Bitmap { get; }

        public int Width => Bitmap.PixelSize.Width;
        public int Height => Bitmap.PixelSize.Height;

        internal Sprite(Bitmap bitmap, float scale = 1)
        {
            if (scale <= 0)
                throw new ArgumentOutOfRangeException(nameof(scale));
            Bitmap = scale == 1 ? bitmap : bitmap.CreateScaledBitmap(new PixelSize((int)(bitmap.PixelSize.Width * scale), (int)(bitmap.PixelSize.Height * scale)), BitmapInterpolationMode.None);
        }
    }
    #endregion
    
    #region Components
    public class Image : Component
    {
        public Vector2 size = new(100, 100);
        public Sprite sprite = null;
        public Color color = new(255);
    }

    public class Shape2D : Component
    {
        List<Vector2> vertices = new() { new(0, 83.2f), new(-100, -80), new(100, -80)};
        public Color color = new();

        public void Clear() => vertices.Clear();
        public List<Vector2> GetVertices() => vertices;
        public void AddVertex(Vector2 vertex) => vertices.Add(vertex);
        public void RemoveVertex(int index) => vertices.RemoveAt(Math.Clamp(index, 0, vertices.Count - 1));
    }
    public class Text : Component
    {
        public string text = "Text...";
        public int fontSize = 16;
        public Color color = new(255);
    }
    public class Button : Component
    {
        public Vector2 size = new(100, 100);
        public Color color = new(255);
        
        public Action onClick;
        public Action onPointerEnter;
        public Action onPointerExit;
        
        bool isOn = false;
        bool isEnter = false;

        public override void Update()
        {
            bool _isOn = GUI.isOnUI(transform.position, size, transform.scale);
            if (Input.Input.mouseLeft && _isOn && !isOn) { isOn = true; onClick?.Invoke(); }
            if (!Input.Input.mouseLeft && isOn) { isOn = false; }
                
            if (_isOn && !isEnter) { isEnter = true; onPointerEnter?.Invoke(); }
            if (!_isOn && isEnter) { isEnter = false; onPointerExit?.Invoke(); }
        }
    }
    public class Slider : Component
    {
        public Vector2 size = new(200, 20);
        public Color backgroundColor = new(255, 100);
        public Color fillColor = new(160);
        public Color handleColor = new(200);
        
        public int valueDecimals = 2;
        
        public Action<float> onValueChanged;

        bool isHolding = false;
        
        float minRange = 0;
        float maxRange = 10;
        float value = 5;
        
        public float GetMin() => minRange;
        public float GetMax() => maxRange;
        public float GetValue() => value;

        public float SetMin(float min) => minRange = min;
        public float SetMax(float max) => maxRange = max;
        public float SetValue(float value) => this.value = Math.Clamp(value, minRange, maxRange);
        float oldValue = 0;
        public override void Update()
        {
            Vector2Int mp = Input.Input.mousePosition;
            double mouseMin = transform.position.x - size.x / 2 * transform.scale.x;
            double mouseMax = transform.position.x + size.x / 2 * transform.scale.x;
            float valMouse = QMath.Round(QMath.Remap(mp.x, (float)mouseMin, (float)mouseMax, minRange, maxRange), valueDecimals);
            if (GUI.isOnUI(transform.position, size, transform.scale) && Input.Input.mouseLeft && !isHolding) { isHolding = true; }
            if (!Input.Input.mouseLeft && isHolding) { isHolding = false; }
            if (isHolding) { SetValue(valMouse); if (value != oldValue) 
                { oldValue = value; onValueChanged?.Invoke(value); } }
        }
    }
    public class Dropdown : Component
    {
        public Vector2 size = new(200, 30);
        public Color color = new(255);
        
        public int labelFontSize = 16;
        public Color labelFontColor = new(0);

        public Vector2 optionSize = new(180, 20);
        public Color optionColor = new(200);
        public int optionFontSize = 12;
        public Color optionFontColor = new(0);

        public float optionsDistance = 5;

        public int option = 0;

        public List<string> options = new() { "Option A", "Option B", "Option C" };

        bool isOn = false;
        public bool isOpened = false;

        public void Clear() => options.Clear();
        public void AddOption(string option) => options.Add(option);
        public void RemoveOption(int index) => options.RemoveAt(Math.Clamp(index, 0, options.Count - 1));

        public override void Update()
        {
            bool _isOn = GUI.isOnUI(transform.position, size, transform.scale);
            if (Input.Input.mouseLeft && _isOn && !isOn) { isOn = true; isOpened = !isOpened; }
            if (!Input.Input.mouseLeft && _isOn && isOn) { isOn = false; }
        }
    }
    public class InputField : Component
    {
        public Vector2 size = new(200, 30);
        public Color color = new();
        
        public string text = "";
        public Color textColor = new(0);
        public int textFontSize = 16;
        
        public string labelText = "Enter text...";
        public Color labelTextColor = new(0, 150);
        public float labelTextFontSize = 16;

        bool isTexting = false;

        public override void Update()
        {
            bool _isOn = GUI.isOnUI(transform.position, size, transform.scale);
            if (Input.Input.mouseLeft && _isOn && !isTexting) { isTexting = true; Input.Input.sKey = string.Empty; }
            if (Input.Input.mouseLeft && !_isOn && isTexting) { isTexting = false; }
            
            if (isTexting && Input.Input.ReadDown(Key.Backspace) && text.Length > 0) { text = text[..^1]; }
            if (isTexting && Input.Input.sKey.Length > 0) { text += Input.Input.sKey; Input.Input.sKey = string.Empty; }
        }
    }
    #endregion
    
}

namespace QEngine.Animations
{
    public class Animation
    {
        List<Sprite> Frames = new();
        int framesPerSecond = 1;
        public void AddFrame(Sprite frame) => Frames.Add(frame);
        public void AddFrames(List<Sprite> frames) { foreach (var f in frames) Frames.Add(f); }
        public Sprite GetFrame(int index) => Frames[Math.Clamp(index, 0, Frames.Count - 1)];
        public int GetFPS() => framesPerSecond;
        public void SetFPS(int fps) => framesPerSecond = fps;
        public List<Sprite> GetFrames() => Frames;
        public int GetFramesCount() => Frames.Count;
        public void Clear() => Frames.Clear();
        public void RemoveFrame(int index) => Frames.RemoveAt(Math.Clamp(index, 0, Frames.Count - 1));
    }
    public class Animator : Component
    {
        public bool isPaused = false;
        Animation current = null;
        int currentFrame = 0;
        
        float time = 0;
        
        Dictionary<string, Animation> animations = new();
        
        public Animation GetAnimation(string name) => animations[name];
        public Animation AddAnimation(string name) { animations.Add(name, new()); return animations[name]; }
        public void RemoveAnimation(string name) => animations.Remove(name);
        public void AddFrame(string name, Sprite frame) => animations[name].AddFrame(frame);
        public void AddFrames(string name, List<Sprite> frames) => animations[name].AddFrames(frames);
        
        public void Play(string name) { if (current != animations[name]) current = animations[name]; isPaused = false; }
        public void Pause() => isPaused = true;
        public void UnPause() => isPaused = false;
        public void Reset() => currentFrame = 0;
        
        public override void Update()
        {
            if (current == null || isPaused) return;
            time += 1 / 60f;
            float timeToNext = 1f / current.GetFPS();
            if (time >= timeToNext)
            {
                currentFrame++; time -= timeToNext;
                if (currentFrame >= current.GetFrames().Count) { currentFrame = 0; }
                if (gameObject.TryGetComponent(out Image img))
                { img.sprite = current.GetFrame(currentFrame); }
            }
        }
    }
}

namespace QEngine.Mathematics
{
    public static class QMath
    {
        public static float Remap(float value, float inMin, float inMax, float outMin, float outMax) => (value - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
        public static float Round(float value, int decimals) => MathF.Round(value, decimals);
        public static Vector2 Round(Vector2 value, int decimals) => new(Round(value.x, decimals), Round(value.y, decimals));
        
        #region Lerps
        public static float Lerp(float a, float b, float t, int decimals = 4) => Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        public static Vector2 Lerp(Vector2 a, Vector2 b, float t, int decimals = 4) => Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        #endregion
    }
}

namespace QEngine.Input
{
    public enum Key
    {
        None,
        A, B, C, D, E, F, G, H, I, J, K, L, M, 
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        
        D1, D2, D3, D4, D5, D6, D7, D8, D9, D0,
        
        Tilde, /* ` */ Minus, /* - */ Plus, /* + */ BracketLeft, /* [ */ BracketRight, /* ] */ 
        Backslash, /* \ */ Semicolon, /* ; */ Quote, /* ' */ Comma, /* , */ Period, /* . */Slash, /* / */ 
        
        NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, 
        NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
        NumPadAdd, NumPadSubstract, NumPadMultiply, NumPadDivide,
        NumPadDecimal, NumPadEnter,
        
        Escape, Tab, CapsLock, ShiftLeft, ShiftRight, ControlLeft, ControlRight, AltLeft, AltRight, Space, Enter, Backspace,
        
        Up, Down, Left, Right,
        
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        
        Insert, Delete, Home, End, PageUp, PageDown
    }

    public class AxisDataInt
    {
        List<Key> Left = new(), Right = new();
        public AxisDataInt(Key left, Key right) { Left.Add(left); Right.Add(right); }
        public AxisDataInt(List<Key> left, List<Key> right) { Left = left; Right = right; }
        public void AddLeft(Key key) => Left.Add(key); public void AddRight(Key key) => Right.Add(key);
        public int Get()
        {
            int v = 0;
            bool left = false, right = false;
            foreach (var l in Left) { if(!left && Input.Read(l)) left = true; }
            foreach (var r in Right) { if(!right && Input.Read(r)) right = true; }
            if (left) { v--; } if (right) { v++; }
            return v;
        }
    }
    public class AxisDataVector2
    {
        List<Key> Up = new(), Down = new(), Left = new(), Right = new();
        public AxisDataVector2(Key up, Key bottom, Key left, Key right)
        { Up.Add(up); Down.Add(bottom); Left.Add(left); Right.Add(right); }
        public AxisDataVector2(List<Key> up, List<Key> bottom, List<Key> left, List<Key> right) 
        { Up = up; Down = bottom; Left = left; Right = right; }
        public void AddLeft(Key key) => Left.Add(key); public void AddRight(Key key) => Right.Add(key);
        public void AddUp(Key key) => Up.Add(key); public void AddBottom(Key key) => Down.Add(key);
        public Vector2 Get()
        {
            Vector2 v = new();
            bool left = false, right = false, up = false, down = false;
            foreach (var u in Up) { if(!up && Input.Read(u)) up = true; }
            foreach (var d in Down) { if(!down && Input.Read(d)) down = true; }
            foreach (var l in Left) { if(!left && Input.Read(l)) left = true; }
            foreach (var r in Right) { if(!right && Input.Read(r)) right = true; }
            if (down) { v.y--; } if (up) { v.y++; }
            if (left) { v.x--; } if (right) { v.x++; }
            return v;
        }
    }
    public static class Input
    {
        public static Vector2Int mousePosition = new();
        
        public static bool mouseLeft = false;
        
        static Dictionary<Key, bool> keys = new();
        static Dictionary<Key, bool> keysPrev = new();

        static Dictionary<string, AxisDataInt> axisesInt = new();
        static Dictionary<string, AxisDataVector2> axisesV2 = new();

        public static string sKey = "";

        public static Action<Key> onKeyDown;
        public static Action<Key> onKeyUp;
        
        static Input()
        {
            keys = new Dictionary<Key, bool>();
            foreach (Key k in Enum.GetValues(typeof(Key)))
                keys[k] = false;
            keysPrev = new(keys);
        }
        public static void Update() => keysPrev = new Dictionary<Key, bool>(keys);
        
        public static bool Read(Key key) => keys[key];
        public static bool ReadDown(Key k)
            => Read(k) && (!keysPrev.TryGetValue(k, out var p) || !p);

        public static void AddAxisInt(string name, Key left, Key right) => axisesInt.Add(name, new(left, right));
        public static void AddAxisInt(string name, List<Key> left, List<Key> right) => axisesInt.Add(name, new(left, right));
        public static int GetAxisInt(string name) => axisesInt.TryGetValue(name, out var v) ? v.Get() : 0;

        public static void AddAxisVector2(string name, Key up, Key bottom, Key left, Key right) => 
            axisesV2.Add(name, new(up, bottom, left, right));
        public static void AddAxisVector2(string name, List<Key> up, List<Key> bottom, List<Key> left, List<Key> right) => 
            axisesV2.Add(name, new(up, bottom, left, right));
        public static Vector2 GetAxisVector2(string name) => axisesV2.TryGetValue(name, out var v) ? v.Get() : new();

        internal static void SetTextInput(object? sender, TextInputEventArgs e) { if (!string.IsNullOrEmpty(e.Text)) sKey = e.Text; }
        internal static void SetKeyDown(Key key) { keys[key] = true; onKeyDown?.Invoke(key); }
        internal static void SetKeyUp(Key key) { keys[key] = false; onKeyUp?.Invoke(key); }
    }
    
    internal static class KeyMapper
    {
        private static readonly Dictionary<Avalonia.Input.Key, Key> _manual = new()
        {
            { Avalonia.Input.Key.Oem3, Key.Tilde },
            { Avalonia.Input.Key.OemMinus, Key.Minus },
            { Avalonia.Input.Key.OemPlus, Key.Plus },
            { Avalonia.Input.Key.Oem4, Key.BracketLeft },
            { Avalonia.Input.Key.Oem6, Key.BracketRight },
            { Avalonia.Input.Key.Oem5, Key.Backslash },
            { Avalonia.Input.Key.Oem1, Key.Semicolon },
            { Avalonia.Input.Key.Oem7, Key.Quote },
            { Avalonia.Input.Key.OemComma, Key.Comma },
            { Avalonia.Input.Key.OemPeriod, Key.Period },
            { Avalonia.Input.Key.Oem2, Key.Slash },
            { Avalonia.Input.Key.Back, Key.Backspace }
        };

        public static bool TryMap(Avalonia.Input.Key aKey, out Key key)
        {
            if (_manual.TryGetValue(aKey, out key))
                return true;

            return Enum.TryParse(aKey.ToString(), out key);
        }
    }
}

namespace QEngine.FileSave
{
    public static class FileSave
    {
        public static void Write(string path, string? content) => File.WriteAllText(path, content);
        public static string Read(string path) => File.ReadAllText(path);
    }
}
