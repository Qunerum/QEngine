using System;
using System.Collections.Generic;
using System.Linq;
// ReSharper disable All

namespace QEngine
{
    #region Variables
    /// <summary> Represents a 2D vector using float precision. Used for world positions, velocity, and directions. </summary>
    public struct Vector2
    {
        /// <summary> The X float of the vector. </summary>
        public float x;
        /// <summary> The Y float of the vector. </summary>
        public float y;
        /// <summary> Initializes a new instance of the <see cref="Vector2"/> struct with all components set to zero. </summary>
        public Vector2() { x = y = 0; }
        /// <summary> Initializes a new instance of the <see cref="Vector2"/> struct with the specified x and y values. </summary>
        /// <param name="x">The X value.</param>
        /// <param name="y">The Y value.</param>
        public Vector2(float x, float y) { this.x = x; this.y = y; }

        /// <summary> Returns a formatted string representation of this vector. </summary>
        /// <returns> A string in the format "(x, y)". </returns>
        public override string ToString() => $"({x}, {y})";
        public static Vector2 operator +(Vector2 a, Vector2 b) => new(a.x + b.x, a.y + b.y);
        public static Vector2 operator -(Vector2 a, Vector2 b) => new(a.x - b.x, a.y - b.y);
        public static Vector2 operator *(Vector2 a, Vector2 b) => new(a.x * b.x, a.y * b.y);
        public static Vector2 operator *(Vector2 a, float b) => new(a.x * b, a.y * b);
        public static Vector2 operator /(Vector2 a, Vector2 b) => new(a.x / b.x, a.y / b.y);
        public static Vector2 operator /(Vector2 a, float b) => new(a.x / b, a.y / b);
        public static bool operator ==(Vector2 a, Vector2 b) => a.x == b.x && a.y == b.y;
        public static bool operator !=(Vector2 a, Vector2 b) => a.x != b.x && a.y != b.y;
        
        public static Vector2 operator -(Vector2 v) => new(-v.x, -v.y);
        public override bool Equals(object? obj) => obj is Vector2 v && this == v;
        public override int GetHashCode() => HashCode.Combine(x, y);
        
        public static float Distance(Vector2 a, Vector2 b)
        {
            float diffX = a.x - b.x, diffY = a.y - b.y;
            return MathF.Sqrt(diffX * diffX + diffY * diffY);
        }
        public static Vector3 toVector3(Vector2 vector) => new(vector.x, vector.y, 0);
        public Vector3 toVector3() => new(x, y, 0);
        public static Vector4 toVector4(Vector2 vector) => new(vector.x, vector.y, 0, 0);
        public Vector4 toVector4() => new(x, y, 0, 0);
    }
    /// <summary> Represents a 2D vector using integer precision. Ideal for grid coordinates and screen resolutions. </summary>
    public struct Vector2Int
    {
        /// <summary> The X int of the vector. </summary>
        public int x;
        /// <summary> The Y int of the vector. </summary>
        public int y;
        /// <summary> Initializes a new instance of the <see cref="Vector2Int"/> struct with all components set to zero. </summary>
        public Vector2Int() { x = y = 0; }
        /// <summary> Initializes a new instance of the <see cref="Vector2Int"/> struct with the specified x and y values. </summary>
        /// <param name="x">The X value.</param>
        /// <param name="y">The Y value.</param>
        public Vector2Int(int x, int y) { this.x = x; this.y = y; }
        /// <summary> Returns true if X and Y are exactly zero. </summary>
        public bool isZero => x == 0 && y == 0;
        /// <summary> Returns a formatted string representation of this vector. </summary>
        /// <returns> A string in the format "(x, y)". </returns>
        public override string ToString() => $"({x}, {y})";
        public static Vector2Int operator +(Vector2Int a, Vector2Int b) => new(a.x + b.x, a.y + b.y);
        public static Vector2Int operator -(Vector2Int a, Vector2Int b) => new(a.x - b.x, a.y - b.y);
        public static Vector2Int operator *(Vector2Int a, Vector2Int b) => new(a.x * b.x, a.y * b.y);
        public static Vector2Int operator *(Vector2Int a, int b) => new(a.x * b, a.y * b);
        public static bool operator ==(Vector2Int a, Vector2Int b) => a.x == b.x && a.y == b.y;
        public static bool operator !=(Vector2Int a, Vector2Int b) => a.x != b.x && a.y != b.y;
        public override bool Equals(object? obj) => obj is Vector2Int v && this == v;
        public override int GetHashCode() => HashCode.Combine(x, y);
        
        public static float Distance(Vector2Int a, Vector2Int b)
        {
            float diffX = a.x - b.x, diffY = a.y - b.y;
            return MathF.Sqrt(diffX * diffX + diffY * diffY);
        }
        
        public static Vector3Int toVector3Int(Vector2Int vector) => new(vector.x, vector.y, 0);
        public Vector3Int toVector3Int() => new(x, y, 0);
        public static Vector4Int toVector4Int(Vector2Int vector) => new(vector.x, vector.y, 0, 0);
        public Vector4Int toVector4Int() => new(x, y, 0, 0);
    }
    public struct Vector3
    {
        /// <summary> The X float of the vector. </summary>
        public float x;
        /// <summary> The Y float of the vector. </summary>
        public float y;
        /// <summary> The Z float of the vector. </summary>
        public float z;
        /// <summary> Initializes a new instance of the <see cref="Vector3"/> struct with all components set to zero. </summary>
        public Vector3() { x = y = z = 0; }
        public Vector3(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }
        /// <summary> Returns a formatted string representation of this vector. </summary>
        /// <returns> A string in the format "(x, y, z)". </returns>
        public override string ToString() => $"({x}, {y}, {z})";
        
        public static Vector3 operator +(Vector3 a, Vector3 b) => new(a.x + b.x, a.y + b.y, a.z + b.z);
        public static Vector3 operator -(Vector3 a, Vector3 b) => new(a.x - b.x, a.y - b.y, a.z - b.z);
        public static Vector3 operator *(Vector3 a, Vector3 b) => new(a.x * b.x, a.y * b.y, a.z * b.z);
        public static Vector3 operator *(Vector3 a, float b) => new(a.x * b, a.y * b, a.z * b);
        public static Vector3 operator /(Vector3 a, Vector3 b) => new(a.x / b.x, a.y / b.y, a.z / b.z);
        public static Vector3 operator /(Vector3 a, float b) => new(a.x / b, a.y / b, a.z / b);
        public static bool operator ==(Vector3 a, Vector3 b) => a.x == b.x && a.y == b.y && a.z == b.z;
        public static bool operator !=(Vector3 a, Vector3 b) => a.x != b.x && a.y != b.y && a.z != b.z;
        public override bool Equals(object? obj) => obj is Vector3 v && this == v;
        public override int GetHashCode() => HashCode.Combine(x, y, z);
        
        public static float Distance(Vector3 a, Vector3 b)
        {
            float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
            return MathF.Sqrt(dx * dx + dy * dy + dz * dz);
        }

        public static Vector4 toVector4(Vector3 vector) => new(vector.x, vector.y, vector.z, 0);
        public Vector4 toVector4() => new(x, y, z, 0);
    }
    public struct Vector3Int
    {
        /// <summary> The X int of the vector. </summary>
        public int x;
        /// <summary> The Y int of the vector. </summary>
        public int y;
        /// <summary> The Z int of the vector. </summary>
        public int z;
        /// <summary> Initializes a new instance of the <see cref="Vector3Int"/> struct with all components set to zero. </summary>
        public Vector3Int() { x = y = z = 0; }
        public Vector3Int(int x, int y, int z) { this.x = x; this.y = y; this.z = z; }
        /// <summary> Returns true if X , Y and Z are exactly zero. </summary>
        public bool isZero => x == 0 && y == 0 && z == 0;
        /// <summary> Returns a formatted string representation of this vector. </summary>
        /// <returns> A string in the format "(x, y, z)". </returns>
        public override string ToString() => $"({x}, {y}, {z})";
        public static Vector3Int operator +(Vector3Int a, Vector3Int b) => new(a.x + b.x, a.y + b.y, a.z + b.z);
        public static Vector3Int operator -(Vector3Int a, Vector3Int b) => new(a.x - b.x, a.y - b.y, a.z - b.z);
        public static Vector3Int operator *(Vector3Int a, Vector3Int b) => new(a.x * b.x, a.y * b.y, a.z * b.z);
        public static Vector3Int operator *(Vector3Int a, int b) => new(a.x * b, a.y * b, a.z * b);
        public static bool operator ==(Vector3Int a, Vector3Int b) => a.x == b.x && a.y == b.y && a.z == b.z;
        public static bool operator !=(Vector3Int a, Vector3Int b) => a.x != b.x && a.y != b.y && a.z != b.z;
        public override bool Equals(object? obj) => obj is Vector3Int v && this == v;
        public override int GetHashCode() => HashCode.Combine(x, y, z);
        
        public static float Distance(Vector3Int a, Vector3Int b)
        {
            float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
            return MathF.Sqrt(dx * dx + dy * dy + dz * dz);
        }
        
        public static Vector4Int toVector4Int(Vector3Int vector) => new(vector.x, vector.y, vector.z, 0);
        public Vector4Int toVector4Int() => new(x, y, z, 0);
    }
    public struct Vector4
    {
        /// <summary> The X float of the vector. </summary>
        public float x;
        /// <summary> The Y float of the vector. </summary>
        public float y;
        /// <summary> The Z float of the vector. </summary>
        public float z;
        /// <summary> The W float of the vector. </summary>
        public float w;
        /// <summary> Initializes a new instance of the <see cref="Vector4"/> struct with all components set to zero. </summary>
        public Vector4() { x = y = z = w = 0; }
        public Vector4(float x, float y, float z, float w) { this.x = x; this.y = y; this.z = z; this.w = w; }
        /// <summary> Returns a formatted string representation of this vector. </summary>
        /// <returns> A string in the format "(x, y, z, w)". </returns>
        public override string ToString() => $"({x}, {y}, {z}, {w})";
        
        public static Vector4 operator +(Vector4 a, Vector4 b) => new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        public static Vector4 operator -(Vector4 a, Vector4 b) => new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        public static Vector4 operator *(Vector4 a, Vector4 b) => new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        public static Vector4 operator *(Vector4 a, float b) => new(a.x * b, a.y * b, a.z * b, a.w * b);
        public static Vector4 operator /(Vector4 a, Vector4 b) => new(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
        public static Vector4 operator /(Vector4 a, float b) => new(a.x / b, a.y / b, a.z / b, a.w / b);
        public static bool operator ==(Vector4 a, Vector4 b) => a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
        public static bool operator !=(Vector4 a, Vector4 b) => a.x != b.x && a.y != b.y && a.z != b.z && a.w != b.w;
        public override bool Equals(object? obj) => obj is Vector4 v && this == v;
        public override int GetHashCode() => HashCode.Combine(x, y, z, w);
    }
    public struct Vector4Int
    {
        /// <summary> The X int of the vector. </summary>
        public int x;
        /// <summary> The Y int of the vector. </summary>
        public int y;
        /// <summary> The Z int of the vector. </summary>
        public int z;
        /// <summary> The W int of the vector. </summary>
        public int w;
        /// <summary> Initializes a new instance of the <see cref="Vector4Int"/> struct with all components set to zero. </summary>
        public Vector4Int() { x = y = z = w = 0; }
        public Vector4Int(int x, int y, int z, int w) { this.x = x; this.y = y; this.z = z; this.w = w; }
        /// <summary> Returns true if X , Y , Z and W are exactly zero. </summary>
        public bool isZero => x == 0 && y == 0 && z == 0 && w == 0;
        /// <summary> Returns a formatted string representation of this vector. </summary>
        /// <returns> A string in the format "(x, y, z, w)". </returns>
        public override string ToString() => $"({x}, {y}, {z}, {w})";

        public static Vector4Int operator +(Vector4Int a, Vector4Int b) => new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        public static Vector4Int operator -(Vector4Int a, Vector4Int b) => new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        public static Vector4Int operator *(Vector4Int a, Vector4Int b) => new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        public static Vector4Int operator *(Vector4Int a, int b) => new(a.x * b, a.y * b, a.z * b, a.w * b);
        public static bool operator ==(Vector4Int a, Vector4Int b) => a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
        public static bool operator !=(Vector4Int a, Vector4Int b) => a.x != b.x && a.y != b.y && a.z != b.z && a.w != b.w;
        public override bool Equals(object? obj) => obj is Vector4Int v && this == v;
        public override int GetHashCode() => HashCode.Combine(x, y, z, w);
    }
    /// <summary> 
    /// Defines the position, rotation, and scale of an object in 2D space. 
    /// Every GameObject has a Transform attached by default.
    /// </summary>
    public class Transform2D
    {
        /// <summary> The world space position of the GameObject in 2D. </summary>
        public Vector2 position = new();
        //public float rotation = 0;
    }
    public class Transform
    {
        /// <summary> The world space position of the GameObject in 3D. </summary>
        public Vector3 position = new();
        //public float rotation = 0;
    }
    #endregion
    
    #region Global
    /// <summary> 
    /// Acts as a container for all GameObjects. 
    /// Manages object lifecycle, camera state, and scene-wide updates.
    /// </summary>
    public class QEScene
    {
        /// <summary> The display name of the scene. </summary>
        public string name = "QE Scene";
        readonly List<GameObject> _objects = new();
        public void AddGameObject(GameObject obj) { Logger.Log($"Adding GameObject '{obj.name}'"); _objects.Add(obj); }
        public GameObject[] GetObjects() => _objects.ToArray();
        public void Clear() => _objects.Clear();
        public GameObject? GetGameObject(string name) =>_objects.FirstOrDefault(o => o.name == name);
        
        /// <summary> Initalizes scene logic. Called once before the first update. </summary>
        public virtual void Init() {}
        /// <summary> Propagates the update call to all GameObjects in the scene. </summary>
        public void Update() { foreach (var obj in _objects) obj.Update(); }
        public void FixedUpdate() { foreach (var obj in _objects) obj.FixedUpdate(); }
    }
    /// <summary> 
    /// The fundamental object in the QEngine. 
    /// Everything that exists in the scene is a GameObject.
    /// </summary>
    public sealed class GameObject
    {
        /// <summary> The identification name of the object. </summary>
        public string name = "";
        /// <summary> Shortcut to the Transform component for positioning. </summary>
        public Transform2D transform = new();
        readonly List<Component> _components = new();
        
        public GameObject(string name = "") { this.name = name; SceneManager.actualScene?.AddGameObject(this); }
        public Component[] GetComponents() => _components.ToArray();
        /// <summary> 
        /// Adds a component of type <typeparamref name="T"/> to the GameObject. 
        /// If the component already exists, it returns the existing one.
        /// </summary>
        public T? AddComponent<T>() where T : Component, new()
        {
            var c = new T { gameObject = this, transform = transform };
            if (!_components.Contains(c))
            {
                _components.Add(c);
                c.Init();
                Logger.Log($"Adding component '{c}' to '{name}'");
                return c;
            } else { return GetComponent<T>(); }
        }
        /// <summary> Returns the first component of type <typeparamref name="T"/> found. </summary>
        public T? GetComponent<T>() where T : Component
            => _components.OfType<T>().FirstOrDefault();
        /// <summary> Tries to get the component of type <typeparamref name="T"/>, returns true if found. </summary>
        public bool TryGetComponent<T>(out T component) where T : Component
        {
            foreach (var c in _components) { if (c is T t) { component = t; return true; } }
            component = null!;
            return false;
        }
        
        internal void Update() { foreach (var c in _components) c.Update(); }
        internal void FixedUpdate() { foreach (var c in _components) c.FixedUpdate(); }
    }
    /// <summary> 
    /// Base class for everything attached to GameObjects. 
    /// Inherit from this to create custom logic or data containers.
    /// </summary>
    public class Component
    {
        /// <summary> The GameObject this component is attached to. </summary>
        public GameObject? gameObject { get; internal set; } = null;
        /// <summary> The Transform of the associated GameObject. </summary>
        public Transform2D? transform { get; internal set; } = null;
        public virtual void Init() {}
        public virtual void Update() {}
        public virtual void FixedUpdate() {}
    }
    /// <summary> Interface for components that can be drawn on the screen. </summary>
    public interface IRenderable
    {
        /// <summary> 
        /// If true, the object is rendered in Screen Space (ignores Camera position and zoom). 
        /// Useful for HUD, menus, and UI elements.
        /// </summary>
        bool isUI { get; set; }
        /// <summary> Called during the rendering phase. </summary>
        void Draw();
    }

    /// <summary> 
    /// Specialized component for writing game scripts. 
    /// Provides easy access to the active scene.
    /// </summary>
    public class QEScript : Component
    {
        /// <summary> Reference to the currently active scene. </summary>
        public QEScene? scene => SceneManager.actualScene;
    }
    #endregion
}
