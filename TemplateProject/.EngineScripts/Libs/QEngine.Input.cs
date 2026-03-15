using System;
using System.Collections.Generic;

using Veldrid;
using Veldrid.Sdl2;

namespace QEngine.Input
{
    /// <summary> Comprehensive list of supported keyboard keys, mapped from SDL2 input events. </summary>
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
    /// <summary> Manages a 1D axis (e.g., Horizontal) using two sets of keys for negative and positive directions. </summary>
    public class AxisDataInt
    {
        List<Key> Left = new(), Right = new();
        public AxisDataInt(Key left, Key right) { Left.Add(left); Right.Add(right); }
        public AxisDataInt(List<Key> left, List<Key> right) { Left = left; Right = right; }
        public void AddLeft(Key key) => Left.Add(key); public void AddRight(Key key) => Right.Add(key);
        /// <summary> Returns -1 if left is pressed, 1 if right is pressed, or 0 if both/none. </summary>
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
    /// <summary> 
    /// Manages a 2D axis (e.g., Movement) using four sets of keys. 
    /// Useful for top-down or platformer movement vectors.
    /// </summary>
    public class AxisDataVector2
    {
        List<Key> Up = new(), Down = new(), Left = new(), Right = new();
        public AxisDataVector2(Key up, Key bottom, Key left, Key right)
        { Up.Add(up); Down.Add(bottom); Left.Add(left); Right.Add(right); }
        public AxisDataVector2(List<Key> up, List<Key> bottom, List<Key> left, List<Key> right) 
        { Up = up; Down = bottom; Left = left; Right = right; }
        public void AddLeft(Key key) => Left.Add(key); public void AddRight(Key key) => Right.Add(key);
        public void AddUp(Key key) => Up.Add(key); public void AddBottom(Key key) => Down.Add(key);
        /// <summary> Returns a normalized-like Vector2 based on the state of Up/Down/Left/Right keys. </summary>
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
    /// <summary> 
    /// Static manager for keyboard and mouse states. 
    /// Interfaces directly with the Sdl2Window to capture hardware events.
    /// </summary>
    public static class Input
    {
        /// <summary> True if the left mouse button is currently held down. </summary>
        public static bool mouseLeft = false;
        /// <summary> True if the right mouse button is currently held down. </summary>
        public static bool mouseRight = false;
        /// <summary> Set of currently pressed keys. Used for polling with <see cref="Read"/>. </summary>
        public static HashSet<Key> keys = new();

        static Dictionary<string, AxisDataInt> axisesInt = new();
        static Dictionary<string, AxisDataVector2> axisesV2 = new();
        /// <summary> Triggered once when a key is first pressed. </summary>
        public static Action<Key>? onKeyDown;
        /// <summary> Triggered once when a key is released. </summary>
        public static Action<Key>? onKeyUp;
        /// <summary> String buffer for text input (used by InputField). </summary>
        public static string data = "";
        /// <summary> Binds SDL2 window events to the Input manager. </summary>
        public static void Init(Sdl2Window window)
        {
            window.KeyDown += OnKeyDownInternal;
            window.KeyUp += OnKeyUpInternal;

            window.MouseMove += OnMouseMove;
            window.MouseDown += OnMouseDown;
            window.MouseUp += OnMouseUp;
        }
        /// <summary> Returns true if the specified key is currently being held. </summary>
        public static bool Read(Key key) => keys.Contains(key);
        /// <summary> Registers a 1D integer axis (e.g., "Horizontal") with a single key for each direction. </summary>
        /// <param name="name">Unique identifier for the axis.</param>
        /// <param name="left">Key that decreases the axis value.</param>
        /// <param name="right">Key that increases the axis value.</param>
        public static void AddAxisInt(string name, Key left, Key right)
            => axisesInt[name] = new AxisDataInt(left, right);
        /// <summary> 
        /// Registers a 1D integer axis with multiple keys assigned to each direction. 
        /// Allows for alternative control schemes (e.g., A and LeftArrow for the same axis). 
        /// </summary>
        public static void AddAxisInt(string name, List<Key> left, List<Key> right)
            => axisesInt[name] = new AxisDataInt(left, right);
        /// <summary> Retrieves the current state of a registered 1D axis. Returns -1, 0, or 1. </summary>
        /// <returns> The calculated axis value, or 0 if the axis name is not found. </returns>
        public static int GetAxisInt(string name)
            => axisesInt.TryGetValue(name, out var v) ? v.Get() : 0;
        /// <summary> Registers a 2D axis (e.g., "Movement") to be polled later. </summary>
        public static void AddAxisVector2(string name, Key up, Key bottom, Key left, Key right)
            => axisesV2[name] = new AxisDataVector2(up, bottom, left, right);
        /// <summary> Registers a 2D vector axis (e.g., "Movement") with multiple keys for each of the four directions. </summary>
        /// <param name="name">Unique identifier for the axis.</param>
        /// <param name="up">Keys for the positive Y direction.</param>
        /// <param name="bottom">Keys for the negative Y direction.</param>
        /// <param name="left">Keys for the negative X direction.</param>
        /// <param name="right">Keys for the positive X direction.</param>
        public static void AddAxisVector2(string name, List<Key> up, List<Key> bottom, List<Key> left, List<Key> right)
            => axisesV2[name] = new AxisDataVector2(up, bottom, left, right);
        /// <summary> Retrieves the calculated Vector2 from a named axis. </summary>
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
        /// <summary> Internally converts Veldrid's key format to QEngine's Key enum. </summary>
        static bool TryConvertKey(Veldrid.Key vk, out Key key)
        {
            string name = vk.ToString();
            if (name == "BackSpace") name = "Backspace";
            if (name == "Return") name = "Enter";
            return Enum.TryParse(name, out key);
        }
    }
    /// <summary> Global state of the system cursor, including its world-space converted position. </summary>
    public static class Cursor
    {
        /// <summary> The current mouse position, adjusted to the game resolution and center-origin. </summary>
        public static Vector2Int position = new();
        /// <summary> Toggles the visibility of the hardware cursor. </summary>
        public static bool isVisible = true;
        
    }
}
