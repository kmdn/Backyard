require 'digest'
require 'open3'

AVROBJCOPY = "/usr/bin/avr-objcopy"
AVROBJCOPYOPTS = "-I ihex -O binary"

class SketchesController < ApplicationController
  before_action :set_sketch, only: [:show, :edit, :update, :destroy]

  # GET /sketches
  # GET /sketches.json
  def index
    @sketches = Sketch.all
  end

  # GET /sketches/1
  # GET /sketches/1.json
  def show
    respond_to do |format|
      format.json { render :show, status: :created, location: @sketch}
    end
  end

  # GET /sketches/new
  def new
    @sketch = Sketch.new
    @patterns = Component.select(:id, :name).where(:category => "pattern")
  end

  # GET /sketches/1/edit
  def edit
    @patterns = Component.select(:id, :name, :pretty_name, :description).where(:category => "pattern")
  end

  # POST /sketches
  # POST /sketches.json
  def create
    @sketch = Sketch.new(sketch_params)
    @sketch.create_sketch
    @sketch.build_sketch

    respond_to do |format|
      if @sketch.save
        format.html { redirect_to @sketch, notice: "Sketch was successfully created. Fingerprint #{@sketch.sha256}, compile size: #{@sketch.size}" }
        format.json { render :show, status: :created, location: @sketch }
      else
        format.html { render :new }
        format.json { render json: @sketch.errors, status: :unprocessable_entity }
      end
    end
  end

  # PATCH/PUT /sketches/1
  # PATCH/PUT /sketches/1.json
  def update
    respond_to do |format|
      if @sketch.update(sketch_params)
        format.html { redirect_to @sketch, notice: 'Sketch was successfully updated.' }
        format.json { render :show, status: :ok, location: @sketch }
      else
        format.html { render :edit }
        format.json { render json: @sketch.errors, status: :unprocessable_entity }
      end
    end
  end

  # DELETE /sketches/1
  # DELETE /sketches/1.json
  def destroy
    @sketch.destroy
    respond_to do |format|
      format.html { redirect_to sketches_url, notice: 'Sketch was successfully destroyed.' }
      format.json { head :no_content }
    end
  end

  def find
    if params[:hex]
      hex = Tempfile.new(['firmware', 'hex'])
      hex.write(params[:hex])

      bin = Tempfile.new(['firmware', 'bin'])

      Open3.capture3("#{AVROBJCOPY} #{AVROBJCOPYOPTS} #{hex.path} #{bin.path}")
      
      binary = File.open(bin.path, "rb") { |file|
        file.read
      }

      Sketch.where("size is not null and sha256 is not null").each do |s|
        if (Digest::SHA256.new.update(binary[0 .. (s.size-1)]) == s.sha256)
          @sketch = Sketch.find(s.id)
          respond_to do |format|
            format.json { render :show, status: :ok, location: @sketch }
          end
        end
      end

      hex.close
      hex.unlink
      bin.close
      bin.unlink

    end  
  end
    

  private
    # Use callbacks to share common setup or constraints between actions.
    def set_sketch
      @sketch = Sketch.find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def sketch_params
      params[:sketch].permit(:config)
    end

    def get_token
      date = Time.now.strftime("%Y-%m-%d")
      token = "#{date}-" + SecureRandom.hex(6)
    token
  end
end