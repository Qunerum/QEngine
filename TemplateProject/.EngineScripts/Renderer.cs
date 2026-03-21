using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Numerics;
using System.Runtime.InteropServices;
using QEngine;
using QEngine.GUI;
using QEngine.Text;

using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;

using Veldrid;
using Veldrid.SPIRV;

using Image = SixLabors.ImageSharp.Image;
using Vector2 = QEngine.Vector2;
using Vector3 = QEngine.Vector3;
using Vector4 = QEngine.Vector4;
// ReSharper disable All

struct SceneMatrices
{
    public Matrix4x4 Projection;
    public Matrix4x4 View;
    public System.Numerics.Vector4 LightDir;
}

struct ObjectMatrices { public Matrix4x4 Model; }
public struct Vertex3D
{
    public Vector3 Position;
    public Vector3 Normal;
    public Vector4 Color;

    public Vertex3D(Vector3 pos, Vector3 norm, Vector4 col)
    {
        Position = pos;
        Normal = norm;
        Color = col;
    }
    public static uint SizeInBytes => 40; 
}

struct Vertex
{
    public Vector2 Position;
    public Vector4 Color;

    public Vertex(Vector2 pos, Vector4 col)
    {
        Position = pos;
        Color = col;
    }

    public static uint SizeInBytes => (2 + 4) * 8;
}
struct SpriteVertex
{
    public Vector2 Position;
    public Vector2 UV;
    public Vector4 Color;
    
    public SpriteVertex(Vector2 p, Vector2 uv, Vector4 c)
    {
        Position = p;
        UV = uv;
        Color = c;
    }

    public static uint SizeInBytes => (2 + 2 + 4) * 4;
}

namespace QEngine.Dev.Renderer
{
    public static class Atlas
    {
    public static Texture? atlasTexture;
    public static TextureView? atlasView;
    
        static byte[]? pixels;
        static Vector2Int size = new();
        static Vector2Int cursor = new();
        static int rowHeight = 0;

        static List<(Sprite sprite, Vector2Int pos)> sprites = new();
        static int maxSlotX = 8;
        static int slotX = 0;

        public static void AddFont(Sprite sprite)
        {
            Console.WriteLine("Adding font to Atlas");
            sprites.Add((sprite, cursor));
            cursor.y += sprite.Height;
            if (cursor.y + rowHeight > size.y) size.y = cursor.y + rowHeight;
            if (sprite.Width > size.x) size.x = sprite.Width;
        }
        public static void AddImage(Sprite sprite)
        {
            Console.WriteLine("Adding Sprite to Atlas");
            if (slotX >= maxSlotX)
            {
                cursor.x = 0;
                cursor.y += rowHeight;
                rowHeight = 0;
                slotX = 0;
            }
            sprites.Add((sprite, cursor));

            cursor.x += sprite.Width;
            if (sprite.Height > rowHeight)
                rowHeight = sprite.Height;
            slotX++;

            if (cursor.x > size.x) size.x = cursor.x;
            if (cursor.y + rowHeight > size.y) size.y = cursor.y + rowHeight;
        }

        public static void Init(out Vector2Int s)
        {
            pixels = new byte[size.x * size.y * 4];
            int bpp = 4;
            foreach (var entry in sprites)
            {
                Sprite sprite = entry.sprite;
                Vector2Int pos = entry.pos;
                for (int y = 0; y < sprite.Height; y++)
                {
                    int atlasStart = ((pos.y + y) * size.x + pos.x) * bpp;
                    int spriteStart = y * sprite.Width * bpp;
                    Array.Copy(sprite.pixels, spriteStart,
                        pixels, atlasStart, sprite.Width * bpp);
                }
                sprite.uv = new Vector4(
                    pos.x / (float)size.x,
                    pos.y / (float)size.y,
                    (pos.x + sprite.Width) / (float)size.x,
                    (pos.y + sprite.Height) / (float)size.y
                );
                Console.WriteLine($"Calculate sprite in Atlas: {sprite.uv}");
            }
            s = size;
            Save();
        }
        
        public static void CreateAtlasTexture(GraphicsDevice gd)
        {
            TextureDescription desc = TextureDescription.Texture2D(
                (uint)size.x, (uint)size.y, 1, 1,
                PixelFormat.R8_G8_B8_A8_UNorm, 
                TextureUsage.Sampled);

            atlasTexture = gd.ResourceFactory.CreateTexture(desc);

            // upload pixels
            gd.UpdateTexture(atlasTexture, pixels, 0, 0, 0, (uint)size.x, (uint)size.y, 1, 0, 0);

            atlasView = gd.ResourceFactory.CreateTextureView(atlasTexture);
        }
        static void Save(string fileName = "Atlas.png")
        {
            Directory.CreateDirectory(Path.Combine(Directory.GetCurrentDirectory(), "Assets"));
            using var img = Image.LoadPixelData<Rgba32>(pixels, size.x, size.y);
            img.Save(Path.Combine(Directory.GetCurrentDirectory(), "Assets", fileName));
        }
    }
    // ================= DATA =================
    /// <summary> Data container for a shape submitted for batch rendering. </summary>
    public struct BatchedShape
    {
        public Vector2 pixelCenter;
        public Vector2[] localPositions;
        public ushort[] indices;
        public Vector4 color;
        public bool isUI;
    }
    /// <summary> Data container for a sprite submitted for batch rendering. </summary>
    public struct BatchedSprite
    {
        public Sprite sprite;
        public Vector4 uv;
        public Vector2 position;
        public Vector2 size;
        public Vector4 color;
        public bool isUI;
    }
    struct BatchedGeometry
    {
        public Vector3 center;
        public Vector3 rotation;
        public Vector3[] Positions;
        public ushort[] Indices;
        public Vector4 Color;
    }

    // ================= RENDERER =================
    public static class QRenderer
    {
        static GraphicsDevice? gd;
        static CommandList? cl;
        static ResourceFactory? factory;

        static Pipeline? shapePipeline;
        static Pipeline? spritePipeline;
        static Pipeline? pipeline3D;

        static DeviceBuffer? shapeVB;
        static DeviceBuffer? shapeIB;

        static DeviceBuffer? spriteVB;
        static DeviceBuffer? spriteIB;

        static Sampler? sampler;
        
        static DeviceBuffer? _testVB;
        
        static DeviceBuffer? _sceneBuffer;
        static DeviceBuffer? _modelBuffer;
        static ResourceSet? _sceneSet;
        static ResourceSet? _modelSet;
        
        static List<BatchedShape> shapesThisFrame = new();
        static List<BatchedSprite> spritesThisFrame = new();
        static List<BatchedGeometry> geometryThisFrame = new();

        // ================= INIT =================
        public static void Init(GraphicsDevice device)
        {
            gd = device;
            factory = gd.ResourceFactory;
            cl = gd.ResourceFactory.CreateCommandList();

            shapeVB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * Vertex.SizeInBytes, BufferUsage.VertexBuffer | BufferUsage.Dynamic));
            shapeIB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * sizeof(ushort), BufferUsage.IndexBuffer | BufferUsage.Dynamic));

            spriteVB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * SpriteVertex.SizeInBytes, BufferUsage.VertexBuffer | BufferUsage.Dynamic));
            spriteIB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * sizeof(ushort), BufferUsage.IndexBuffer | BufferUsage.Dynamic));

            sampler = gd.ResourceFactory.CreateSampler(new SamplerDescription(
                SamplerAddressMode.Clamp,
                SamplerAddressMode.Clamp,
                SamplerAddressMode.Clamp,
                SamplerFilter.MinPoint_MagPoint_MipPoint,
                ComparisonKind.Never,
                0, 0, 0, 0,
                SamplerBorderColor.OpaqueBlack));

            atlasLayout = gd.ResourceFactory.CreateResourceLayout(
                new ResourceLayoutDescription(
                    new ResourceLayoutElementDescription("mySampler", ResourceKind.Sampler, ShaderStages.Fragment),
                    new ResourceLayoutElementDescription("myTexture", ResourceKind.TextureReadOnly, ShaderStages.Fragment)
                ));

            // -------- 3D PIPELINE --------
            // 1. Shadery
            var shader3D = factory.CreateFromSpirv(
                new ShaderDescription(ShaderStages.Vertex, Encoding.UTF8.GetBytes(Shader3DVS), "main"),
                new ShaderDescription(ShaderStages.Fragment, Encoding.UTF8.GetBytes(Shader3DFS), "main"));

            // 2. BUFORY I LAYOUTY (Tego brakowało!)
            _sceneBuffer = factory.CreateBuffer(new BufferDescription(
                (uint)Marshal.SizeOf<SceneMatrices>(), BufferUsage.UniformBuffer));
            
            _modelBuffer = factory.CreateBuffer(new BufferDescription(
                (uint)Marshal.SizeOf<ObjectMatrices>(), BufferUsage.UniformBuffer));

            // Definiujemy strukturę zasobów (Layouty)
            ResourceLayout sceneLayout = factory.CreateResourceLayout(new ResourceLayoutDescription(
                new ResourceLayoutElementDescription("SceneBuffer", ResourceKind.UniformBuffer, ShaderStages.Vertex)));

            ResourceLayout modelLayout = factory.CreateResourceLayout(new ResourceLayoutDescription(
                new ResourceLayoutElementDescription("ModelBuffer", ResourceKind.UniformBuffer, ShaderStages.Vertex)));

            // Tworzymy powiązania (Sety) - przypisujemy konkretne bufory do layoutów
            _sceneSet = factory.CreateResourceSet(new ResourceSetDescription(sceneLayout, _sceneBuffer));
            _modelSet = factory.CreateResourceSet(new ResourceSetDescription(modelLayout, _modelBuffer));

            // 3. Konfiguracja Pipeline
            GraphicsPipelineDescription pipelineDescription = new GraphicsPipelineDescription();
            pipelineDescription.BlendState = BlendStateDescription.SingleOverrideBlend;
            pipelineDescription.DepthStencilState = new DepthStencilStateDescription(
                depthTestEnabled: true, depthWriteEnabled: true, comparisonKind: ComparisonKind.LessEqual);
            pipelineDescription.RasterizerState = new RasterizerStateDescription(
                cullMode: FaceCullMode.None, fillMode: PolygonFillMode.Solid, frontFace: FrontFace.Clockwise, 
                depthClipEnabled: true, scissorTestEnabled: false);
            pipelineDescription.PrimitiveTopology = PrimitiveTopology.TriangleList;

            pipelineDescription.ShaderSet = new ShaderSetDescription(
                new[] {
                    new VertexLayoutDescription(
                        new VertexElementDescription("Position", VertexElementSemantic.Position, VertexElementFormat.Float3),
                        new VertexElementDescription("Normal", VertexElementSemantic.Normal, VertexElementFormat.Float3), // Dodano to!
                        new VertexElementDescription("Color", VertexElementSemantic.Color, VertexElementFormat.Float4))
                }, shader3D);

            // Teraz zmienne sceneLayout i modelLayout już istnieją!
            pipelineDescription.ResourceLayouts = new[] { sceneLayout, modelLayout };
            pipelineDescription.Outputs = gd.SwapchainFramebuffer.OutputDescription;

            pipeline3D = factory.CreateGraphicsPipeline(pipelineDescription);
            // -------- SHAPE PIPELINE --------
            var shapeShaders = gd.ResourceFactory.CreateFromSpirv(
                new ShaderDescription(ShaderStages.Vertex, Encoding.UTF8.GetBytes(ShapeVS), "main"),
                new ShaderDescription(ShaderStages.Fragment, Encoding.UTF8.GetBytes(ShapeFS), "main"));

            shapePipeline = gd.ResourceFactory.CreateGraphicsPipeline(
                new GraphicsPipelineDescription
                {
                    BlendState = BlendStateDescription.SingleAlphaBlend,
                    DepthStencilState = DepthStencilStateDescription.Disabled,
                    RasterizerState = RasterizerStateDescription.CullNone,
                    PrimitiveTopology = PrimitiveTopology.TriangleList,
                    ShaderSet = new ShaderSetDescription(
                        new[]
                        {
                            new VertexLayoutDescription(
                                new VertexElementDescription("Position", VertexElementSemantic.Position, VertexElementFormat.Float2),
                                new VertexElementDescription("Color", VertexElementSemantic.Color, VertexElementFormat.Float4))
                        },
                        shapeShaders),
                    ResourceLayouts = Array.Empty<ResourceLayout>(),
                    Outputs = gd.SwapchainFramebuffer.OutputDescription
                });

            // -------- SPRITE PIPELINE --------

            var spriteShaders = gd.ResourceFactory.CreateFromSpirv(
                new ShaderDescription(ShaderStages.Vertex, Encoding.UTF8.GetBytes(SpriteVS), "main"),
                new ShaderDescription(ShaderStages.Fragment, Encoding.UTF8.GetBytes(SpriteFS), "main"));

            spritePipeline = gd.ResourceFactory.CreateGraphicsPipeline(
                new GraphicsPipelineDescription
                {
                    BlendState = BlendStateDescription.SingleAlphaBlend,
                    DepthStencilState = DepthStencilStateDescription.Disabled,
                    RasterizerState = RasterizerStateDescription.CullNone,
                    PrimitiveTopology = PrimitiveTopology.TriangleList,
                    ShaderSet = new ShaderSetDescription(
                        new[]
                        {
                            new VertexLayoutDescription(
                                new VertexElementDescription("Position", VertexElementSemantic.Position, VertexElementFormat.Float2),
                                new VertexElementDescription("UV", VertexElementSemantic.TextureCoordinate, VertexElementFormat.Float2),
                                new VertexElementDescription("Color", VertexElementSemantic.Color, VertexElementFormat.Float4))
                        },
                        spriteShaders),
                    ResourceLayouts = new[] { atlasLayout },
                    Outputs = gd.SwapchainFramebuffer.OutputDescription
                });
        }

        public static void Begin()
        {
            cl?.Begin();
            cl?.SetFramebuffer(gd?.SwapchainFramebuffer);
            cl?.ClearColorTarget(0, Game.background.toRgba());
            cl?.ClearDepthStencil(1f); 

            shapesThisFrame.Clear();
            spritesThisFrame.Clear();
            geometryThisFrame.Clear();
        }

        public static void End()
        {
            FlushGeometry();
            FlushShapes();
            FlushSprites();

            cl?.End();
            gd?.SubmitCommands(cl);
            gd?.SwapBuffers();
        }
        public static void CreateAtlasSampler(GraphicsDevice gd)
        {
            sampler = gd.ResourceFactory.CreateSampler(new SamplerDescription(
                SamplerAddressMode.Clamp, SamplerAddressMode.Clamp, SamplerAddressMode.Clamp,
                SamplerFilter.MinPoint_MagPoint_MipPoint, ComparisonKind.Never,
                0, 0, 0, 0,
                SamplerBorderColor.OpaqueBlack));
        }
        static ResourceLayout? atlasLayout;
        static ResourceSet? atlasSet;
        public static void CreateAtlasResourceSet(GraphicsDevice gd)
        => atlasSet = gd.ResourceFactory.CreateResourceSet(new ResourceSetDescription(atlasLayout, 
            sampler, Atlas.atlasView));
        // =======================================================================================================================================
        // ================= API ===================================================================================================================
        // =======================================================================================================================================
        
        #region Drawing 3D
        public static void DrawGeometry(Vector3 center, Vector3 rotation, Vector3[] verts, ushort[] inds, Vector4 color)
        {
            geometryThisFrame.Add(new BatchedGeometry()
            {
                center = center,
                rotation = rotation,
                Positions = verts,
                Indices = inds,
                Color = color
            });
        }

        public static void DrawCube(Vector3 center, Vector3 rotation, Vector3 size, Vector4 color)
        {
            float x = size.x / 2, y = size.y / 2, z = size.z / 2;
            DrawGeometry(center, rotation, [
                new(-x, y, -z), new(x, y, -z),
                new(x, y, z), new(-x, y, z),
                new(-x, -y, -z), new(x, -y, -z),
                new(x, -y, z), new(-x, -y, z)
            ], [0, 1, 2,  0, 2, 3, 4, 6, 5,  4, 7, 6,
                0, 5, 1,  0, 4, 5, 3, 2, 6,  3, 6, 7,
                1, 5, 6,  1, 6, 2, 0, 3, 7,  0, 7, 4], color);
        }
        #endregion
        
        #region Drawing 2D
        public static void DrawShape(Vector2 center, Vector2[] verts, ushort[] inds, Vector4 color, bool isUI = false)
        {
            shapesThisFrame.Add(new BatchedShape
            {
                pixelCenter = center,
                localPositions = verts,
                indices = inds,
                color = color,
                isUI = isUI
            });
        }
        
        static ushort[] quad = { 0, 1, 2, 0, 2, 3 };
        public static void DrawLine(Vector2 a, Vector2 b, float thickness, Vector4 color, bool isUI = false)
        {
            float length = Vector2.Distance(a, b);
            if (length <= 0) return;
            Vector2 dir = new(b.x - a.x, b.y - a.y), perp = new Vector2(-dir.y / length, dir.x / length) * (thickness * 0.5f),
                p1 = perp, p2 = dir + perp, p3 = dir - perp, p4 = -perp;
            DrawShape(a, [p1, p2, p3, p4], quad, color, isUI);
        }
        public static void DrawBox(Vector2 center, float size, Vector4 color, bool isUI = false)
            => DrawShape(center, [new Vector2(-size, -size) / 2, new Vector2(size, -size) / 2, new Vector2(size, size) / 2, new Vector2(-size, size) / 2], quad, color, isUI); 
        public static void DrawBox(Vector2 center, Vector2 size, Vector4 color, bool isUI = false)
            => DrawShape(center, [new Vector2(-size.x, -size.y) / 2, new Vector2(size.x, -size.y) / 2, size / 2, new Vector2(-size.x, size.y) / 2], quad, color, isUI);

        public static void DrawWireBox(Vector2 center, Vector2 size, float thickness, Vector4 color, bool isUI = false)
        {
            Vector2 a = new(center.x - size.x / 2, center.y + size.y / 2), b = new(center.x + size.x / 2, center.y + size.y / 2),
                c = new(center.x - size.x / 2, center.y - size.y / 2), d = new(center.x + size.x / 2, center.y - size.y / 2);
            DrawLine(a, b, thickness, color, isUI); DrawLine(b, d, thickness, color, isUI);
            DrawLine(d, c, thickness, color, isUI); DrawLine(c, a, thickness, color, isUI);
            DrawLine(a, d, thickness, color, isUI); DrawLine(b, c, thickness, color, isUI);
        }

        public static void DrawSprite(Sprite sprite, Vector4 uv, Vector2 pos, Vector2 size, Vector4 color, bool isUI = false)
        {
            spritesThisFrame.Add(new BatchedSprite
            {
                sprite = sprite,
                uv = uv,
                position = pos,
                size = size,
                color = color,
                isUI = isUI
            });
        }
        
        public static void DrawText(Font font, float space, string text, Vector2 position, int fontSize, Vector4 color, bool isUI = false)
        {
            float scale = fontSize / font.defaultFontSize;
            Vector2 cursor = new();
            foreach (char c in text)
            {
                if (c == '\n')
                {
                    cursor.x = 0;
                    cursor.y -= font.charSize.y * scale;
                    continue;
                }

                if (c == ' ')
                {
                    cursor.x += font.charSize.x * scale;
                    continue;
                }
                if (!font.glyphs.TryGetValue(c, out Glyph g))
                    continue;
                Vector2 size = new Vector2(font.charSize.x, font.charSize.y) * scale;
                DrawSprite(
                    font.texture,
                    new Vector4(g.uvMin.x, g.uvMin.y, g.uvMax.x, g.uvMax.y),
                    position + cursor + new Vector2(g.thick * scale, size.y) / 2,
                    size, color, isUI
                );
                cursor.x += space + g.thick * scale;
            }
        }
        #endregion
        
        // =======================================================================================================================================
        // ============= API END ===================================================================================================================
        // =======================================================================================================================================
        
        static DeviceBuffer? EnsureBuffer(
            DeviceBuffer? buffer,
            uint requiredBytes,
            BufferUsage usage)
        {
            if (buffer == null || buffer.SizeInBytes < requiredBytes)
            {
                buffer?.Dispose();

                uint newSize = Math.Max(requiredBytes, requiredBytes * 2);

                buffer = gd?.ResourceFactory.CreateBuffer(
                    new BufferDescription(newSize, usage));
            }
            return buffer;
        }
        // ================= FLUSH =================
        static void FlushShapes()
        {
            if (shapesThisFrame.Count == 0) return;
            int totalVerts = 0, totalIndices = 0;

            foreach (var s in shapesThisFrame)
            {
                totalVerts += s.localPositions.Length;
                totalIndices += s.indices.Length;
            }

            uint requiredVB = (uint)(totalVerts * Vertex.SizeInBytes), 
                requiredIB = (uint)(totalIndices * sizeof(ushort));
            
            shapeVB = EnsureBuffer(shapeVB, requiredVB, BufferUsage.VertexBuffer | BufferUsage.Dynamic);
            shapeIB = EnsureBuffer(shapeIB, requiredIB, BufferUsage.IndexBuffer | BufferUsage.Dynamic);
            
            List<Vertex> v = new(totalVerts);
            List<ushort> i = new(totalIndices);
            
            foreach (var s in shapesThisFrame)
            {
                ushort baseIndex = (ushort)v.Count;
                foreach (var p in s.localPositions)
                {
                    Vector2 pos = s.pixelCenter + p;
                    Vector2 finalNDC = s.isUI ? Camera.ScreenToNDC(pos) : Camera.WorldToNDC(pos);
                    v.Add(new Vertex(finalNDC, s.color));
                }
                foreach (var idx in s.indices) i.Add((ushort)(idx + baseIndex));
            }

            gd?.UpdateBuffer(shapeVB, 0, v.ToArray());
            gd?.UpdateBuffer(shapeIB, 0, i.ToArray());

            cl?.SetPipeline(shapePipeline);
            cl?.SetVertexBuffer(0, shapeVB);
            cl?.SetIndexBuffer(shapeIB, IndexFormat.UInt16);
            cl?.DrawIndexed((uint)i.Count, 1, 0, 0, 0);
        }

        static void FlushSprites()
        {
            int spriteCount = spritesThisFrame.Count;
            if (spriteCount == 0) return;

            uint requiredVB = (uint)(spriteCount * 4 * SpriteVertex.SizeInBytes), 
                requiredIB = (uint)(spriteCount * 6 * sizeof(ushort));

            spriteVB = EnsureBuffer(spriteVB, requiredVB, BufferUsage.VertexBuffer | BufferUsage.Dynamic);
            spriteIB = EnsureBuffer(spriteIB, requiredIB, BufferUsage.IndexBuffer | BufferUsage.Dynamic);

            SpriteVertex[] verts = new SpriteVertex[spriteCount * 4];
            ushort[] indices = new ushort[spriteCount * 6];

            int vo = 0, io = 0;
            foreach (var s in spritesThisFrame) WriteQuad(verts, indices, ref vo, ref io, s);

            gd?.UpdateBuffer(spriteVB, 0, verts);
            gd?.UpdateBuffer(spriteIB, 0, indices);

            cl?.SetPipeline(spritePipeline);
            cl?.SetVertexBuffer(0, spriteVB);
            cl?.SetIndexBuffer(spriteIB, IndexFormat.UInt16);
            cl?.SetGraphicsResourceSet(0, atlasSet);
            cl?.DrawIndexed((uint)indices.Length, 1, 0, 0, 0);

            spritesThisFrame.Clear();
        }

        static List<Vertex3D> allVertices = new();
        static void FlushGeometry()
        {
            if (geometryThisFrame.Count == 0) return;
            allVertices.Clear();

            foreach (var batch in geometryThisFrame)
            {
                // Tworzymy macierz modelu dla tego konkretnego obiektu
                Matrix4x4 model = Matrix4x4.CreateRotationX(batch.rotation.x * (MathF.PI / 180f)) *
                                  Matrix4x4.CreateRotationY(batch.rotation.y * (MathF.PI / 180f)) *
                                  Matrix4x4.CreateRotationZ(batch.rotation.z * (MathF.PI / 180f)) *
                                  Matrix4x4.CreateTranslation(batch.center.x, batch.center.y, batch.center.z);

                for (int i = 0; i < batch.Indices.Length; i += 3)
                {
                    // Pobieramy 3 punkty trójkąta
                    Vector3 v1 = batch.Positions[batch.Indices[i]];
                    Vector3 v2 = batch.Positions[batch.Indices[i + 1]];
                    Vector3 v3 = batch.Positions[batch.Indices[i + 2]];

                    // Transformujemy punkty macierzą na CPU
                    System.Numerics.Vector3 tv1 = System.Numerics.Vector3.Transform(new(v1.x, v1.y, v1.z), model);
                    System.Numerics.Vector3 tv2 = System.Numerics.Vector3.Transform(new(v2.x, v2.y, v2.z), model);
                    System.Numerics.Vector3 tv3 = System.Numerics.Vector3.Transform(new(v3.x, v3.y, v3.z), model);

                    // Liczymy normalną dla przetransformowanych punktów
                    var n = System.Numerics.Vector3.Normalize(System.Numerics.Vector3.Cross(tv2 - tv1, tv3 - tv1));
                    Vector3 normal = new Vector3(n.X, n.Y, n.Z);

                    allVertices.Add(new Vertex3D(new(tv1.X, tv1.Y, tv1.Z), normal, batch.Color));
                    allVertices.Add(new Vertex3D(new(tv2.X, tv2.Y, tv2.Z), normal, batch.Color));
                    allVertices.Add(new Vertex3D(new(tv3.X, tv3.Y, tv3.Z), normal, batch.Color));
                }
            }

            // TERAZ wysyłamy wszystko RAZ
            cl!.SetPipeline(pipeline3D);
            cl.SetGraphicsResourceSet(0, _sceneSet);

            // Ustawiamy model na Identity, bo wierzchołki są już w World Space
            ObjectMatrices identityObj = new ObjectMatrices { Model = Matrix4x4.Identity };
            gd!.UpdateBuffer(_modelBuffer, 0, ref identityObj);
            cl.SetGraphicsResourceSet(1, _modelSet);

            uint totalSize = (uint)(allVertices.Count * Vertex3D.SizeInBytes);
            _testVB = EnsureBuffer(_testVB, totalSize, BufferUsage.VertexBuffer | BufferUsage.Dynamic);
            gd.UpdateBuffer(_testVB, 0, allVertices.ToArray());

            cl.SetVertexBuffer(0, _testVB);
            cl.Draw((uint)allVertices.Count, 1, 0, 0);
        }

        static void WriteQuad(SpriteVertex[] v, ushort[] i, ref int vo, ref int io, BatchedSprite s)
        {
            Vector2 c = s.isUI ? Camera.ScreenToNDC(s.position) : Camera.WorldToNDC(s.position);
            Vector2 h = s.isUI ? Camera.ScreenSizeToNDC(s.size) * 0.5f : Camera.WorldSizeToNDC(s.size) * 0.5f;

            v[vo + 0] = new(c + new Vector2(-h.x, -h.y), new(s.uv.x, s.uv.y), s.color);
            v[vo + 1] = new(c + new Vector2(+h.x, -h.y), new(s.uv.z, s.uv.y), s.color);
            v[vo + 2] = new(c + new Vector2(+h.x, +h.y), new(s.uv.z, s.uv.w), s.color);
            v[vo + 3] = new(c + new Vector2(-h.x, +h.y), new(s.uv.x, s.uv.w), s.color);


            i[io + 0] = (ushort)(vo + 0);
            i[io + 1] = (ushort)(vo + 1);
            i[io + 2] = (ushort)(vo + 2);
            i[io + 3] = (ushort)(vo + 2);
            i[io + 4] = (ushort)(vo + 3);
            i[io + 5] = (ushort)(vo + 0);

            vo += 4;
            io += 6;
        }
        public static void UpdateCamera3D(Vector3 camPos, Vector3 rotation, Vector3 sunPos)
        {
            SceneMatrices sm = new SceneMatrices(); 

            float aspect = (float)Game.resolution.x / Game.resolution.y;
            Matrix4x4 projection = Matrix4x4.CreatePerspectiveFieldOfView(MathF.PI / 4, aspect, 0.1f, 1000f);
            projection.M22 *= -1; 

            float pitch = rotation.x * (MathF.PI / 180f); // Góra/Dół
            float yaw = rotation.y * (MathF.PI / 180f);   // Lewo/Prawo

            System.Numerics.Vector3 forward;
            forward.X = MathF.Cos(pitch) * MathF.Sin(yaw);
            forward.Y = MathF.Sin(pitch);
            forward.Z = MathF.Cos(pitch) * MathF.Cos(yaw);

            System.Numerics.Vector3 cPos = new(camPos.x, camPos.y, camPos.z);
    
            sm.Projection = projection;
            sm.View = Matrix4x4.CreateLookAt(cPos, cPos + forward, System.Numerics.Vector3.UnitY);

            var s = new System.Numerics.Vector4(sunPos.x, sunPos.y, sunPos.z, 0);
            sm.LightDir = System.Numerics.Vector4.Normalize(s); 

            gd?.UpdateBuffer(_sceneBuffer, 0, sm);
        }
        // ================= SHADERS =================
        const string ShapeVS = @"#version 450
layout(location=0) in vec2 Position;
layout(location=1) in vec4 Color;
layout(location=0) out vec4 fsColor;
void main(){ gl_Position=vec4(Position,0,1); fsColor=Color; }";

        const string ShapeFS = @"#version 450
layout(location=0) in vec4 fsColor;
layout(location=0) out vec4 outColor;
void main(){ outColor=fsColor; }";

        const string SpriteVS = @"#version 450
layout(location=0) in vec2 Position;
layout(location=1) in vec2 UV;
layout(location=2) in vec4 Color;
layout(location=0) out vec2 fsUV;
layout(location=1) out vec4 fsColor;
void main(){ gl_Position=vec4(Position,0,1); fsUV=UV; fsColor=Color; }";

        const string SpriteFS = @"#version 450
layout(location=0) in vec2 fsUV;
layout(location=1) in vec4 fsColor;
layout(set=0,binding=0) uniform sampler mySampler;
layout(set=0,binding=1) uniform texture2D myTexture;
layout(location=0) out vec4 outColor;
void main(){ outColor=texture(sampler2D(myTexture,mySampler),fsUV)*fsColor; }";
        
        const string Shader3DVS = @"#version 450
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;

// DANE GLOBALNE (Set 0)
layout(set = 0, binding = 0) uniform SceneBuffer {
    mat4 Projection;
    mat4 View;
    vec3 LightDir;
};

// DANE OBIEKTU (Set 1)
layout(set = 1, binding = 0) uniform ModelBuffer {
    mat4 Model;
};

layout(location = 0) out vec4 fsColor;

void main() {
    // Mnożymy macierze przez pozycję
    gl_Position = Projection * View * Model * vec4(Position, 1.0);
    
    // Obliczamy oświetlenie
    vec3 worldNormal = normalize(mat3(Model) * Normal);
    float diffuse = max(dot(worldNormal, normalize(LightDir)), 0.0);
    float ambient = 0.2;
    
    // Przekazujemy kolor do fragment shadera
    fsColor = vec4(Color.rgb * (diffuse + ambient), Color.a);
}";

        const string Shader3DFS = @"#version 450
layout(location = 0) in vec4 fsColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = fsColor;
}";
    }
}
