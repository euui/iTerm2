#import <Foundation/Foundation.h>

#import "iTermMetalBufferPool.h"
#import "iTermMetalFrameData.h"
#import "iTermPreciseTimer.h"
#import "iTermShaderTypes.h"
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>

NS_ASSUME_NONNULL_BEGIN

// Maybe I could increase this in the future but it's easier to reason about issues during development when it's 1.
// This is exposed because it's used to set the capacity of mixed-size buffer pools.
extern const NSInteger iTermMetalDriverMaximumNumberOfFramesInFlight;

@class iTermMetalRendererTransientState;

NS_CLASS_AVAILABLE(10_11, NA)
@interface iTermRenderConfiguration : NSObject
@property (nonatomic, readonly) vector_uint2 viewportSize;
@property (nonatomic, readonly) CGFloat scale;

- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithViewportSize:(vector_uint2)viewportSize
                               scale:(CGFloat)scale NS_DESIGNATED_INITIALIZER;
@end

NS_CLASS_AVAILABLE(10_11, NA)
@protocol iTermMetalRenderer<NSObject>
@property (nonatomic, readonly) BOOL rendererDisabled;

- (iTermMetalFrameDataStat)createTransientStateStat;
- (void)drawWithRenderEncoder:(id<MTLRenderCommandEncoder>)renderEncoder
               transientState:(__kindof iTermMetalRendererTransientState *)transientState;

- (__kindof iTermMetalRendererTransientState *)createTransientStateForConfiguration:(iTermRenderConfiguration *)configuration
                                                                      commandBuffer:(id<MTLCommandBuffer>)commandBuffer;

@end

NS_CLASS_AVAILABLE(10_11, NA)
@interface iTermMetalRendererTransientState : NSObject
@property (nonatomic, strong, readonly) __kindof iTermRenderConfiguration *configuration;
@property (nonatomic, strong) id<MTLBuffer> vertexBuffer;
@property (nonatomic, readonly) iTermMetalBufferPoolContext *poolContext;

// You don't generally need to assign to this unless you plan to make more than one draw call.
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, readonly) BOOL skipRenderer;

- (instancetype)initWithConfiguration:(__kindof iTermRenderConfiguration *)configuration;
- (instancetype)init NS_UNAVAILABLE;

- (void)measureTimeForStat:(int)index ofBlock:(void (^)(void))block;
- (iTermPreciseTimerStats *)stats;
- (int)numberOfStats;
- (NSString *)nameForStat:(int)i;

@end

NS_CLASS_AVAILABLE(10_11, NA)
@interface iTermMetalRenderer : NSObject

@property (nonatomic, readonly) id<MTLDevice> device;
@property (nonatomic, readonly) Class transientStateClass;
@property (nonatomic, copy) NSString *fragmentFunctionName;

- (nullable instancetype)initWithDevice:(id<MTLDevice>)device;

- (nullable instancetype)initWithDevice:(id<MTLDevice>)device
                     vertexFunctionName:(NSString *)vertexFunctionName
                   fragmentFunctionName:(NSString *)fragmentFunctionName
                               blending:(BOOL)blending
                    transientStateClass:(Class)transientStateClass;

- (instancetype)init NS_UNAVAILABLE;

- (id<MTLRenderPipelineState>)pipelineState;

#pragma mark - For subclasses

- (id<MTLBuffer>)newQuadOfSize:(CGSize)size poolContext:(iTermMetalBufferPoolContext *)poolContext;

// Things in Metal are randomly upside down for no good reason. So make it easy to flip them back.
- (id<MTLBuffer>)newFlippedQuadOfSize:(CGSize)size poolContext:(iTermMetalBufferPoolContext *)poolContext;

- (void)drawWithTransientState:(iTermMetalRendererTransientState *)tState
                 renderEncoder:(id <MTLRenderCommandEncoder>)renderEncoder
              numberOfVertices:(NSInteger)numberOfVertices
                  numberOfPIUs:(NSInteger)numberOfPIUs
                 vertexBuffers:(NSDictionary<NSNumber *, id<MTLBuffer>> *)vertexBuffers
               fragmentBuffers:(NSDictionary<NSNumber *, id<MTLBuffer>> *)fragmentBuffers
                      textures:(NSDictionary<NSNumber *, id<MTLTexture>> *)textures;

- (id<MTLTexture>)textureFromImage:(NSImage *)image;

- (id<MTLRenderPipelineState>)newPipelineWithBlending:(BOOL)blending
                                       vertexFunction:(id<MTLFunction>)vertexFunction
                                     fragmentFunction:(id<MTLFunction>)fragmentFunction;

- (__kindof iTermMetalRendererTransientState *)createTransientStateForConfiguration:(iTermRenderConfiguration *)configuration
                                                                      commandBuffer:(id<MTLCommandBuffer>)commandBuffer;

@end

NS_ASSUME_NONNULL_END