using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;

using QEngine.Dev.Renderer;
using QEngine.GUI;
using QEngine.Input;
using QEngine.Mathematics;

using Veldrid;
using Veldrid.Sdl2;

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

namespace QEngine.Text
{
    public class Font
    {
        public Sprite texture;
        public Dictionary<char, Glyph> glyphs;
        public Vector2Int charSize;
        public float defaultFontSize;

        public void CalcAndSetGlyphs(Vector2Int size, string fontName)
        {
            Console.WriteLine($"Calculating glyphs for '{fontName}'...");
            foreach (var key in glyphs.Keys.ToList())
            {
                Glyph g = glyphs[key]; 
                Vector2Int pos = g.pos * charSize;
                g.SetMinMax(new(
                    pos.x / (float)size.x,
                    pos.y / (float)size.y,
                    (pos.x + charSize.x) / (float)size.x,
                    (pos.y + charSize.y) / (float)size.y));
                glyphs[key] = g;
                Console.WriteLine($"Glyph '{key}' calculated: position: {pos} , uv: {g.uvMin} {g.uvMax}");
            }
        }
    }
    public struct Glyph
    {
        public Vector2Int pos;
        public Vector2 uvMin;
        public Vector2 uvMax;
        public float thick;
        public Glyph(Vector2Int pos, float thick) { this.pos = pos; this.thick = thick; }
        public void SetMinMax(Vector4 uv) { uvMin = new(uv.x, uv.y); uvMax = new (uv.z, uv.w); }
    }
    public class Text : Component, IRenderable
    {
        public string text = "Text...";
        public Font font = Assets.GetFont("Default");
        public int fontSize = 16;
        public float space = 8;
        public Color color = new(255);
        public Vector2 preferedSize = new();
        
        public void Draw()
        {
            if (font == null || font.texture == null || 
                font.texture.pixels == null || font.texture.pixels.Length <= 0) return;
            QRenderer.DrawText(font, space, text, transform.position, fontSize, color.to01());
        }
    }
}

namespace QEngine.GUI
{
    public static class GUI
    {
        public static bool isOnUI(Vector2 center, Vector2 size)
        {
            Vector2 mp = new(Cursor.position.x, Cursor.position.y);
            float halfW = size.x / 2;
            float halfH = size.y / 2;
            return
                mp.x >= center.x - halfW &&
                mp.x <= center.x + halfW &&
                mp.y >= center.y - halfH &&
                mp.y <= center.y + halfH;
        }
    }

    #region Variables
    public struct Color
    {
        public byte r, g, b, a;
        public Color(byte rgb, byte a = 255) { r = rgb; g = rgb; b = rgb; this.a = a; }
        public Color(byte r, byte g, byte b, byte a = 255) { this.r = r; this.g = g; this.b = b; this.a = a; }
        
        public Vector4 to01() => new Vector4(r, g, b, a) / 255f;
        public RgbaFloat toRgba() => new(r/255f, g/255f, b/255f, a/255f);
        
        public static Color Red => new(255, 0, 0);
        public static Color Green => new(0, 255, 0);
        public static Color Blue => new(0, 0, 255);
        public static Color White => new(255);
        public static Color Black => new(0);
    }

    public class Sprite
    {
        public int Width, Height;
        public Vector4 uv;
        public byte[] pixels;
        public Sprite(int Width, int Height, byte[] pxs)
        { this.Width = Width; this.Height = Height; pixels = pxs; }
    }
    #endregion
    
    #region Components
    public class Image : Component, IRenderable
    {
        public Vector2 size = new(100, 100);
        public Sprite sprite;
        public Color color = Color.White;

        public void Draw()
        {
            if (sprite.pixels.Length > 0)
            { QRenderer.DrawSprite(sprite, sprite.uv, transform.position, size, color.to01()); } else
            {
                QRenderer.DrawShape(transform.position,
                    new Vector2[]
                    {
                        new(-size.x / 2, -size.y / 2),
                        new(size.x / 2, -size.y / 2),
                        new(size.x / 2, size.y / 2),
                        new(-size.x / 2, size.y / 2)
                    }, new ushort[]
                    {
                        0, 1, 2,
                        0, 2, 3
                    }, color.to01());
            }
        }
    }

    public class Shape2D : Component, IRenderable
    {
        List<Vector2> vertices = new() { new(0, 83.2f), new(-100, -80), new(100, -80) };
        List<ushort> indices = new() { 0, 2, 1 };
        public Color color = Color.White;

        public void Clear() { vertices.Clear(); indices.Clear(); }

        public List<Vector2> GetVertices() => vertices;
        public void AddVertex(Vector2 vertex) => vertices.Add(vertex);
        public void SetVertices(List<Vector2> vertices) => this.vertices = vertices;
        public void RemoveVertex(int index) => vertices.RemoveAt(Math.Clamp(index, 0, vertices.Count - 1));
        public List<ushort> GetIndices() => indices;
        public void AddIndice(ushort indice) => indices.Add(indice);
        public void SetIndices(List<ushort> indices) => this.indices = indices;
        public void RemoveIndice(int index) => indices.RemoveAt(Math.Clamp(index, 0, vertices.Count - 1));
        
        public void Draw() { QRenderer.DrawShape(transform.position,vertices.ToArray(), indices.ToArray(), color.to01()); }
    }

    public class LineDrawer : Component
    {
        public float lineThickness = 5;
        public Color color = new();
        List<Vector2> points = new() { new(-50, 0), new(50, 0) };
        public void Clear() => points.Clear();
        public List<Vector2> GetPoints() => points;
        public void AddPoint(Vector2 vertex) => points.Add(vertex);
        public void RemovePoint(int index) => points.RemoveAt(Math.Clamp(index, 0, points.Count - 1));
        
        public void Draw()
        {
            
        }
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
            bool _isOn = GUI.isOnUI(transform.position, size);
            if (Input.Input.mouseLeft && _isOn && !isOn) { isOn = true; onClick?.Invoke(); }
            if (!Input.Input.mouseLeft && isOn) { isOn = false; }
                
            if (_isOn && !isEnter) { isEnter = true; onPointerEnter?.Invoke(); }
            if (!_isOn && isEnter) { isEnter = false; onPointerExit?.Invoke(); }
        }
        
        public void Draw()
        {
            
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
            Vector2Int mp = Cursor.position;
            double mouseMin = transform.position.x - size.x / 2;
            double mouseMax = transform.position.x + size.x / 2;
            float valMouse = QMath.Round(QMath.Remap(mp.x, (float)mouseMin, (float)mouseMax, minRange, maxRange), valueDecimals);
            if (GUI.isOnUI(transform.position, size) && Input.Input.mouseLeft && !isHolding) { isHolding = true; }
            if (!Input.Input.mouseLeft && isHolding) { isHolding = false; }
            if (isHolding) { SetValue(valMouse); if (value != oldValue) 
                { oldValue = value; onValueChanged?.Invoke(value); } }
        }
        
        public void Draw()
        {
            
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
            bool _isOn = GUI.isOnUI(transform.position, size);
            if (Input.Input.mouseLeft && _isOn && !isOn) { isOn = true; isOpened = !isOpened; }
            if (!Input.Input.mouseLeft && _isOn && isOn) { isOn = false; }
        }
        
        public void Draw()
        {
            
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

        /*
        public override void Update()
        {
            bool _isOn = GUI.isOnUI(transform.position, size, transform.scale);
            if (Input.Input.mouseLeft && _isOn && !isTexting) { isTexting = true; Input.Input.sKey = string.Empty; }
            if (Input.Input.mouseLeft && !_isOn && isTexting) { isTexting = false; }
            
            if (isTexting && Input.Input.ReadDown(Key.Backspace) && text.Length > 0) { text = text[..^1]; }
            if (isTexting && Input.Input.sKey.Length > 0) { text += Input.Input.sKey; Input.Input.sKey = string.Empty; }
        }
        */
        public void Draw()
        {
            
        }
    }

    #region Particle System
    public class ParticleData
    {
        public Vector2 position = new();
        
        public Color startColor = Color.White;
        public Color color = Color.White;
        
        public float startLifetime = 2;
        public float lifetime = 2;
        
        public float startSpeed = 5;
        public float speed = 5;
        
        public float startSize = 5;
        public float size = 5;
        
        public int rotation = 0;

        public ParticleData() {}
        public ParticleData(ParticleData p)
        {
            startColor = p.startColor;
            color = p.color;

            startLifetime = p.startLifetime;
            lifetime = p.lifetime;

            startSpeed = p.startSpeed;
            speed = p.speed;

            startSize = p.startSize;
            size = p.size;
        }
    }
    public class ParticleSystem : Component, IRenderable
    {
        public bool isPlaying = false;
        public int particlesPerSeconds = 4;
        
        public Color startColor = Color.White;
        public float startLifetime = 2;
        public float startSpeed = 5;
        public float startSize = 5;
        
        public List<Color> colorByLifetime = new();
        public List<float> speedByLifetime = new();
        public List<float> sizeByLifetime = new();

        public int minAngle = 0;
        public int maxAngle = 360;
        
        List<ParticleData> particles = new();
        
        public void Play() => isPlaying = true;
        public void Stop() => isPlaying = false;
        public void Clear() => particles.Clear();
        public void StopImmediate() { particles.Clear(); isPlaying = false; }
        public List<ParticleData> GetParticles() => particles;
        Random r = new();
        public void Emit(int count = 1) { for (int i = 0; i < count; i++) {
                particles.Add(new ParticleData
                {
                    startColor = startColor,
                    color = startColor,

                    startLifetime = startLifetime,
                    lifetime = startLifetime,

                    startSpeed = startSpeed,
                    speed = startSpeed,

                    startSize = startSize,
                    size = startSize,

                    rotation = r.Next(minAngle, maxAngle)
                }); } }
        float time = 0;

        public override void Update()
        {
            if (particlesPerSeconds <= 0)
                return;

            if (maxAngle < 0 || maxAngle > 360)
                maxAngle = Math.Clamp(maxAngle, 0, 360);
            if (minAngle < 0 || minAngle > 360)
                minAngle = Math.Clamp(minAngle, 0, 360);

            if (isPlaying)
            {
                time += Time.deltaTime;
                float timeToSpawn = 1f / particlesPerSeconds;
                while (time >= timeToSpawn)
                {
                    time -= timeToSpawn;
                    Emit();
                }
            }

            if (particles.Count > 0)
            {
                for (int i = particles.Count - 1; i >= 0; i--)
                {
                    var p = particles[i];

                    p.lifetime -= Time.deltaTime;
                    if (p.lifetime <= 0)
                    {
                        particles.RemoveAt(i);
                        continue;
                    }

                    p.position += QMath.AngleToVector(p.rotation) * p.speed * 10f * Time.deltaTime;

                    float lifeT = 1f - p.lifetime / p.startLifetime;
                    lifeT = Math.Clamp(lifeT, 0f, 1f);

                    p.color = EvaluateByLifetime(p.startColor, colorByLifetime, lifeT);
                    p.speed = EvaluateByLifetime(p.startSpeed, speedByLifetime, lifeT);
                    p.size = EvaluateByLifetime(p.startSize, sizeByLifetime, lifeT);
                }
            }
        }
        static float EvaluateByLifetime(float start, List<float> values, float t)
        {
            if (values == null || values.Count == 0)
                return start;

            int total = values.Count + 1;

            float scaled = t * (total - 1);
            int index = (int)scaled;
            float localT = scaled - index;

            float from = index == 0
                ? start
                : values[index - 1];

            float to = index < values.Count
                ? values[index]
                : values[^1];

            return QMath.Lerp(from, to, localT);
        }
        static Color EvaluateByLifetime(Color start, List<Color> values, float t)
        {
            if (values == null || values.Count == 0)
                return start;

            int total = values.Count + 1;

            float scaled = t * (total - 1);
            int index = (int)scaled;
            float localT = scaled - index;

            Color from = index == 0
                ? start
                : values[index - 1];

            Color to = index < values.Count
                ? values[index]
                : values[^1];

            return QMath.Lerp(from, to, localT);
        }

        public void Draw()
        {
            ushort[] us = new ushort[] { 0, 1, 2, 0, 2, 3 };
            foreach (var p in particles)
            {
                QRenderer.DrawShape(transform.position + p.position,
                    new Vector2[]
                    {
                        new(-p.size / 2, -p.size / 2),
                        new(p.size / 2, -p.size / 2),
                        new(p.size / 2, p.size / 2),
                        new(-p.size / 2, p.size / 2)
                    }, us, p.color.to01()); }
        }
    }
    #endregion
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
            time += 1f / 60f;
            float timeToNext = 1f / current.GetFPS();
            while (time >= timeToNext)
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
        
        public static Vector2 AngleToVector(float angleDeg)
        {
            float rad = angleDeg * MathF.PI / 180f, x = MathF.Cos(rad), y = MathF.Sin(rad);
            return new (x, y);
        }
        
        #region Lerps
        public static float Lerp(float a, float b, float t, int decimals = 4) => 
            Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        public static Vector2 Lerp(Vector2 a, Vector2 b, float t, int decimals = 4) => 
            Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        public static Color Lerp(Color a, Color b, float t)
        => new((byte)Math.Clamp(a.r + (b.r - a.r) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.g + (b.g - a.g) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.b + (b.b - a.b) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.a + (b.a - a.a) * Math.Clamp(t, 0, 1), 0, 255));
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
        public static bool mouseLeft = false;
        public static bool mouseRight = false;

        public static HashSet<Key> keys = new();

        static Dictionary<string, AxisDataInt> axisesInt = new();
        static Dictionary<string, AxisDataVector2> axisesV2 = new();

        public static Action<Key>? onKeyDown;
        public static Action<Key>? onKeyUp;

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
            Cursor.position.x = (int)e.MousePosition.X;
            Cursor.position.y = (int)e.MousePosition.Y;
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

namespace QEngine.FileSave
{
    public static class FileSave
    {
        public static void Write(string path, string? content) 
            => File.WriteAllText(path.Replace(";here;", Path.Combine(Directory.GetCurrentDirectory(), "Assets")), content);
        public static string Read(string path) 
            => File.ReadAllText(path.Replace(";here;", Path.Combine(Directory.GetCurrentDirectory(), "Assets")));
    }
}


