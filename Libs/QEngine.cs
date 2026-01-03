using System;
using System.Collections.Generic;
using System.Linq;
using Avalonia;
using Avalonia.Media;
using Avalonia.Media.Imaging;

namespace QEngine
{
    #region Variables
    public class Vector2
    {
        public float x, y;
        public Vector2(float x = 0, float y = 0) { this.x = x; this.y = y; }
        
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
        public string Name = "";
        readonly List<GameObject> _objects = new();
        
        public GameObject CreateObject(string name = "GameObject") { GameObject go = new(name); _objects.Add(go); return go; }
        public List<GameObject> getObjects() => _objects;
        
        public virtual void Init() {}
        public void Update() { foreach (var obj in _objects) obj.Update(); }
    }
    public sealed class GameObject
    {
        public string Name = "";
        public Transform transform = new();
        readonly List<Component> _components = new();
        
        public GameObject(string name = "") => Name = name;
        
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
    public class QEScript : Component { }
    #endregion
}

namespace QEngine.Mathematics
{
    public static class QMath
    {
        public static float Remap(float value, float inMin, float inMax, float outMin, float outMax) => (value - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
        public static float Round(float value, int decimals) => MathF.Round(value, decimals);
    }
}

namespace QEngine.GUI
{
    #region Components
    public class Image : Component
    {
        public Vector2 size = new(100, 100);
        public Sprite sprite = null;
        public Color color = new(255);
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
        public string text = "Button...";
        public int fontSize = 16;
        public Color textColor = new(255);
        public Color color = new(255);

        public bool isOn = false;
        public bool isEnter = false;

        public Action onClick;
        public Action onPointerEnter;
        public Action onPointerExit;
    }
    public class Slider : Component
    {
        public Vector2 size = new(200, 20);
        public Color backgroundColor = new(255, 100);
        public Color fillColor = new(160);
        public Color handleColor = new(200);
        
        public int valueDecimals = 2;
        
        public Action<float> OnValueChanged;

        public bool isHolding = false;
        
        float minRange = 0;
        float maxRange = 10;
        float value = 5;
        
        public float GetMin() => minRange;
        public float GetMax() => maxRange;
        public float GetValue() => value;

        public float SetMin(float min) => minRange = min;
        public float SetMax(float max) => maxRange = max;
        public float SetValue(float value) => this.value = Math.Clamp(value, minRange, maxRange);
    }
    #endregion
    
    #region Variables
    public class Color
    {
        public byte r, g, b, a;
        public SolidColorBrush _clr;

        public Color(byte rgb, byte a = 255)
        {
            r = rgb; g = rgb; b = rgb; this.a = a; 
            _clr = new(new Avalonia.Media.Color(a, rgb, rgb, rgb));
        }

        public Color(byte r, byte g, byte b, byte a = 255)
        {
            this.r = r; this.g = g; this.b = b; this.a = a;
            _clr = new(new Avalonia.Media.Color(a, r, g, b));
        } 
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

            Bitmap = scale == 1
                ? bitmap
                : bitmap.CreateScaledBitmap(
                    new PixelSize(
                        (int)(bitmap.PixelSize.Width * scale),
                        (int)(bitmap.PixelSize.Height * scale)
                    ),
                    BitmapInterpolationMode.None
                );
        }
    }
    #endregion
}

namespace QEngine.Input
{
    public enum Key
    {
        None,
        A, B, C, D, E, F, G, H, I, J, K, L, M, 
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        
        D1, D2, D3, D4, D5, D6, D7, D8, D9, D0,
        
        NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, 
        NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
        NumPadAdd, NumPadSubstract, NumPadMultiply, NumPadDivide,
        NumPadDecimal, NumPadEnter,
        
        Tilde, /* ` */ Minus, /* - */ Plus, /* + */ BracketLeft, /* [ */ BracketRight, /* ] */ 
        Backslash, /* \ */ Semicolon, /* ; */ Quote, /* ' */ Comma, /* , */ Period, /* . */Slash, /* / */ 
        
        Escape, Tab, CapsLock, ShiftLeft, ShiftRight, ControlLeft, ControlRight, AltLeft, AltRight, Space, Enter, Backspace,
        
        Up, Down, Left, Right,
        
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        
        Insert, Delete, Home, End, PageUp, PageDown
    }
    
    public static class Input
    {
        public static Vector2Int mousePosition = new();
        
        public static bool mouseLeft = false;
        
        static Dictionary<Key, bool> keys = new();

        static Input()
        {
            keys = new Dictionary<Key, bool>();
            foreach (Key k in Enum.GetValues(typeof(Key)))
                keys[k] = false;
        }

        public static bool Read(Key key) => keys[key];

        internal static void SetKeyDown(Key key) => keys[key] = true;
        internal static void SetKeyUp(Key key) => keys[key] = false;
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
        };

        public static bool TryMap(Avalonia.Input.Key aKey, out Key key)
        {
            if (_manual.TryGetValue(aKey, out key))
                return true;

            return Enum.TryParse(aKey.ToString(), out key);
        }
    }
}
