using System;
using System.Collections.Generic;

using Veldrid;
using Veldrid.Sdl2;

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
        public static bool mouseLeft = false;
        public static bool mouseRight = false;

        public static HashSet<Key> keys = new();

        static Dictionary<string, AxisDataInt> axisesInt = new();
        static Dictionary<string, AxisDataVector2> axisesV2 = new();

        public static Action<Key>? onKeyDown;
        public static Action<Key>? onKeyUp;

        public static string data = "";

        public static void Init(Sdl2Window window)
        {
            window.KeyDown += OnKeyDownInternal;
            window.KeyUp += OnKeyUpInternal;

            window.MouseMove += OnMouseMove;
            window.MouseDown += OnMouseDown;
            window.MouseUp += OnMouseUp;
        }

        public static bool Read(Key key) => keys.Contains(key);

        public static void AddAxisInt(string name, Key left, Key right)
            => axisesInt[name] = new AxisDataInt(left, right);
        public static void AddAxisInt(string name, List<Key> left, List<Key> right)
            => axisesInt[name] = new AxisDataInt(left, right);
        public static int GetAxisInt(string name)
            => axisesInt.TryGetValue(name, out var v) ? v.Get() : 0;
        public static void AddAxisVector2(string name, Key up, Key bottom, Key left, Key right)
            => axisesV2[name] = new AxisDataVector2(up, bottom, left, right);
        public static void AddAxisVector2(string name, List<Key> up, List<Key> bottom, List<Key> left, List<Key> right)
            => axisesV2[name] = new AxisDataVector2(up, bottom, left, right);
        public static Vector2 GetAxisVector2(string name)
            => axisesV2.TryGetValue(name, out var v) ? v.Get() : new();
        
        static void OnKeyDownInternal(KeyEvent e)
        {
            if (!TryConvertKey(e.Key, out var key))
                return;

            if (keys.Add(key))
                onKeyDown?.Invoke(key);
        }
        static void OnKeyUpInternal(KeyEvent e)
        {
            if (!TryConvertKey(e.Key, out var key))
                return;

            if (keys.Remove(key))
                onKeyUp?.Invoke(key);
        }
        static void OnMouseMove(MouseMoveEventArgs e)
        {
            Cursor.position.x = (int)e.MousePosition.X - Game.resolution.x / 2;
            Cursor.position.y = -(int)e.MousePosition.Y + Game.resolution.y / 2;
        }
        static void OnMouseDown(MouseEvent e)
        {
            if (e.MouseButton == MouseButton.Left) mouseLeft = true;
            if (e.MouseButton == MouseButton.Right) mouseRight = true;
        }
        static void OnMouseUp(MouseEvent e)
        {
            if (e.MouseButton == MouseButton.Left) mouseLeft = false;
            if (e.MouseButton == MouseButton.Right) mouseRight = false;
        }

        static bool TryConvertKey(Veldrid.Key vk, out Key key)
        {
            string name = vk.ToString();
            if (name == "BackSpace") name = "Backspace";
            if (name == "Return") name = "Enter";
            return Enum.TryParse(name, out key);
        }
    }

    public static class Cursor
    {
        public static Vector2Int position = new();
        public static bool isVisible = true;
        
    }
}
