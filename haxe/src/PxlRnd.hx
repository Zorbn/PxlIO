import haxe.Int32;

class PxlRnd {
    public function new(windowName:String, windowWidth:Int32, windowHeight:Int32, viewWidth:Int32, viewHeight:Int32, enableVsync:Bool = true) {
        PxlRndBindings.pxlrnd_create(windowName, windowWidth, windowHeight, viewWidth, viewHeight, enableVsync);
    }

    public function pollEvents():Bool {
        return PxlRndBindings.pxlrnd_poll_events();
    }

    public function getDeltaTime() {
        return PxlRndBindings.pxlrnd_get_delta_time();
    }

    public function beginDrawing() {
        PxlRndBindings.pxlrnd_begin_drawing();
    }

    public function endDrawing() {
        PxlRndBindings.pxlrnd_end_drawing();
    }

    public function setBackgroundColor(r:Single, g:Single, b:Single) {
        PxlRndBindings.pxlrnd_set_background_color(r, g, b);
    }

    public function setScreenBackgroundColor(r:Single, g:Single, b:Single) {
        PxlRndBindings.pxlrnd_set_screen_background_color(r, g, b);
    }

	public function createSpriteBatch(texturePath:String, maxSprites:Int32, smooth:Bool = false, enableBlending:Bool = false):SpriteBatch {
		return new SpriteBatch(PxlRndBindings.pxlrnd_create_sprite_batch(texturePath, maxSprites, smooth, enableBlending));
	}

	public function destroySpriteBatch(spriteBatch:SpriteBatch) {
        PxlRndBindings.pxlrnd_destroy_sprite_batch(spriteBatch.id);
    }

	public function drawSpriteBatch(spriteBatch:SpriteBatch) {
        PxlRndBindings.pxlrnd_draw_sprite_batch(spriteBatch.id);
    }
}