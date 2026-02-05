using System;
using System.Collections.Generic;
using System.Linq;

namespace QEngine
{
    #region Variables
    public struct Vector2
    {
        public float x, y;
        public Vector2() { x = y = 0; }
        public Vector2(float x, float y) { this.x = x; this.y = y; }
        
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

        public static bool operator ==(Vector2 a, Vector2 b)
            => a.x == b.x && a.y == b.y;
        public static bool operator !=(Vector2 a, Vector2 b)
            => a.x != b.x && a.y != b.y;
        
        public override bool Equals(object? obj)
        {
            if (obj is not Vector2 v) return false;
            return this == v;
        }
        public override int GetHashCode()
            => HashCode.Combine(x, y);
    }
    public struct Vector2Int
    {
        public int x, y;
        public Vector2Int() { x = y = 0; }
        public Vector2Int(int x, int y) { this.x = x; this.y = y; }
        
        public bool isZero => x == 0 && y == 0;

        public override string ToString() => $"({x}, {y})";

        public static Vector2Int operator +(Vector2Int a, Vector2Int b)
            => new(a.x + b.x, a.y + b.y);
        
        public static Vector2Int operator -(Vector2Int a, Vector2Int b)
            => new(a.x - b.x, a.y - b.y);
        
        public static Vector2Int operator *(Vector2Int a, Vector2Int b)
            => new(a.x * b.x, a.y * b.y);
        public static Vector2Int operator *(Vector2Int a, int b)
            => new(a.x * b, a.y * b);
        
        public static bool operator ==(Vector2Int a, Vector2Int b)
            => a.x == b.x && a.y == b.y;
        public static bool operator !=(Vector2Int a, Vector2Int b)
            => a.x != b.x && a.y != b.y;
        
        public override bool Equals(object? obj)
        {
            if (obj is not Vector2Int v) return false;
            return this == v;
        }
        public override int GetHashCode()
            => HashCode.Combine(x, y);
    }
    public struct Vector3
    {
        public float x, y, z;
        public Vector3() { x = y = z = 0; }
        public Vector3(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }
        
        public override string ToString() => $"({x}, {y}, {z})";
        
        public static Vector3 operator +(Vector3 a, Vector3 b)
            => new(a.x + b.x, a.y + b.y, a.z + b.z);
        
        public static Vector3 operator -(Vector3 a, Vector3 b)
            => new(a.x - b.x, a.y - b.y, a.z - b.z);
        
        public static Vector3 operator *(Vector3 a, Vector3 b)
            => new(a.x * b.x, a.y * b.y, a.z * b.z);
        public static Vector3 operator *(Vector3 a, float b)
            => new(a.x * b, a.y * b, a.z * b);
        
        public static Vector3 operator /(Vector3 a, Vector3 b)
            => new(a.x / b.x, a.y / b.y, a.z / b.z);
        public static Vector3 operator /(Vector3 a, float b)
            => new(a.x / b, a.y / b, a.z / b);
        
        public static bool operator ==(Vector3 a, Vector3 b)
            => a.x == b.x && a.y == b.y && a.z == b.z;
        public static bool operator !=(Vector3 a, Vector3 b)
            => a.x != b.x && a.y != b.y && a.z != b.z;
        
        public override bool Equals(object? obj)
        {
            if (obj is not Vector3 v) return false;
            return this == v;
        }
        public override int GetHashCode()
            => HashCode.Combine(x, y);
    }
    public struct Vector3Int
    {
        public int x, y, z;
        public Vector3Int() { x = y = z = 0; }
        public Vector3Int(int x, int y, int z) { this.x = x; this.y = y; this.z = z; }
        
        public bool isZero => x == 0 && y == 0 && z == 0;

        public override string ToString() => $"({x}, {y})";

        public static Vector3Int operator +(Vector3Int a, Vector3Int b)
            => new(a.x + b.x, a.y + b.y, a.z + b.z);
        
        public static Vector3Int operator -(Vector3Int a, Vector3Int b)
            => new(a.x - b.x, a.y - b.y, a.z - b.z);
        
        public static Vector3Int operator *(Vector3Int a, Vector3Int b)
            => new(a.x * b.x, a.y * b.y, a.z * b.z);
        public static Vector3Int operator *(Vector3Int a, int b)
            => new(a.x * b, a.y * b, a.z * b);
        
        public static bool operator ==(Vector3Int a, Vector3Int b)
            => a.x == b.x && a.y == b.y && a.z == b.z;
        public static bool operator !=(Vector3Int a, Vector3Int b)
            => a.x != b.x && a.y != b.y && a.z != b.z;
        
        public override bool Equals(object? obj)
        {
            if (obj is not Vector3Int v) return false;
            return this == v;
        }
        public override int GetHashCode()
            => HashCode.Combine(x, y);
    }
    public struct Vector4
    {
        public float x, y, z, w;
        public Vector4() { x = y = z = w = 0; }
        public Vector4(float x, float y, float z, float w) { this.x = x; this.y = y; this.z = z; this.w = w; }
        
        public override string ToString() => $"({x}, {y}, {z}, {w})";
        
        public static Vector4 operator +(Vector4 a, Vector4 b)
            => new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        
        public static Vector4 operator -(Vector4 a, Vector4 b)
            => new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        
        public static Vector4 operator *(Vector4 a, Vector4 b)
            => new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        public static Vector4 operator *(Vector4 a, float b)
            => new(a.x * b, a.y * b, a.z * b, a.w * b);
        
        public static Vector4 operator /(Vector4 a, Vector4 b)
            => new(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
        public static Vector4 operator /(Vector4 a, float b)
            => new(a.x / b, a.y / b, a.z / b, a.w / b);
        
        public static bool operator ==(Vector4 a, Vector4 b)
            => a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
        public static bool operator !=(Vector4 a, Vector4 b)
            => a.x != b.x && a.y != b.y && a.z != b.z && a.w != b.w;
        
        public override bool Equals(object? obj)
        {
            if (obj is not Vector4 v) return false;
            return this == v;
        }
        public override int GetHashCode()
            => HashCode.Combine(x, y);
    }
    public struct Vector4Int
    {
        public int x, y, z, w;
        public Vector4Int() { x = y = z = w = 0; }
        public Vector4Int(int x, int y, int z, int w) { this.x = x; this.y = y; this.z = z; this.w = w; }

        public bool isZero => x == 0 && y == 0 && z == 0 && w == 0;
        
        public override string ToString() => $"({x}, {y}, {z}, {w})";

        public static Vector4Int operator +(Vector4Int a, Vector4Int b)
            => new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        
        public static Vector4Int operator -(Vector4Int a, Vector4Int b)
            => new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        
        public static Vector4Int operator *(Vector4Int a, Vector4Int b)
            => new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        public static Vector4Int operator *(Vector4Int a, int b)
            => new(a.x * b, a.y * b, a.z * b, a.w * b);
        
        public static bool operator ==(Vector4Int a, Vector4Int b)
            => a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
        public static bool operator !=(Vector4Int a, Vector4Int b)
            => a.x != b.x && a.y != b.y && a.z != b.z && a.w != b.w;
        
        public override bool Equals(object? obj)
        {
            if (obj is not Vector4Int v) return false;
            return this == v;
        }
        public override int GetHashCode()
            => HashCode.Combine(x, y);
    }
    
    public class Transform
    {
        public Vector2 position = new();
        //public float rotation = 0;
    }
    public class Transform3D
    {
        public Vector3 position = new();
        //public float rotation = 0;
    }
    #endregion
    
    #region Global
    public class QEScene
    {
        public string name;
        readonly List<GameObject> _objects = new();

        public Vector3 cameraPosition = new();
        public void AddGameObject(GameObject obj) { Console.WriteLine($"Adding GameObject '{obj.name}'"); _objects.Add(obj); }
        public List<GameObject> GetObjects() => _objects;
        
        public void Clear() => _objects.Clear();
        
        public GameObject? GetGameObject(string name) =>_objects.FirstOrDefault(o => o.name == name);
        
        public virtual void Init() {}
        public void Update() { foreach (var obj in _objects) obj.Update(); }
    }
    public sealed class GameObject
    {
        public string name = "";
        public Transform transform = new();
        readonly List<Component> _components = new();
        
        public GameObject(string name = "") { this.name = name; SceneManager.actualScene?.AddGameObject(this); }
        public List<Component> GetComponents() => _components;
        public T AddComponent<T>() where T : Component, new()
        {
            var c = new T { gameObject = this, transform = transform };
            if (!_components.Contains(c))
            {
                _components.Add(c);
                c.Init();
                Console.WriteLine($"Adding component '{c}' to '{name}'");
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
    public interface IRenderable { void Draw(); }
    public class QEScript : Component { public QEScene? scene => SceneManager.actualScene; }
    #endregion
}
