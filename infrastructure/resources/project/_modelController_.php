<?php

namespace App\Http\Controllers;

use App\Models\_modelName_;
use Illuminate\Http\Request;

/**
 * Generate docs: php artisan l5-swagger:generate
 * Deployd  docs: php artisan vendor:publish --provider "L5Swagger\L5SwaggerServiceProvider"
 */

/**
 *
 *
 * @OA\Info(title="Pojects's API", version="0.1")
 *
 *
 * @OA\Get(
 *  path="/api/_model_",
 *  tags={"_modelName_: RO context"},
 *  summary="List all/filtered resources",
 *  description="List resources",
 *  @OA\Response(response="default", description="List all/filtered resources"),
 *  @OA\Parameter(
 *      name="filter[key]",
 *      in="query",
 *      description="key",
 *      required=false,
 *  ),
 *  @OA\Parameter(
 *      name="filter[value]",
 *      in="query",
 *      description="value",
 *      required=false,
 *  ),
 *  @OA\Parameter(
 *      name="filter[options]",
 *      in="query",
 *      description="options",
 *      required=false,
 *  ),
 * )
 *
 * @OA\Get(
 *  path="/api/_model_/{uuid}",
 *  tags={"_modelName_: RO context"},
 *  summary="Show single resource.",
 *  description="Get resource by UUID.",
 *  @OA\Response(response="default", description="Show single resource."),
 *  @OA\Parameter(
 *      name="uuid",
 *      in="path",
 *      description="UUID",
 *      required=true,
 *  ),
 * )
 *
 * @OA\Post(
 *  path="/api/_model_",
 *  summary="Create new resource entry.",
 *  description="Create resource with data",
 *  operationId="_modelEntry__post",
 *  tags={"_modelName_: RW context"},
 *  @OA\RequestBody(
 *      required=true,
 *      description="resource data",
 *      @OA\JsonContent(
 *          required={"key","value","options"},
 *          @OA\Property(property="key", type="string", format="text", example="some_key"),
 *          @OA\Property(property="value", type="string", format="text", example="some_value"),
 *          @OA\Property(property="options", type="string", example="some_options"),
 *      )
 *  ),
 *  @OA\Response(
 *      response=200,
 *      description="Success",
 *      @OA\JsonContent(
 *          @OA\Property(property="status", type="integer", example="1"),
 *          @OA\Property(property="data",   type="object", example="{}"),
 *      )
 *  )
 * )
 *
 * @OA\Patch(
 *  path="/api/_model_/{uuid}",
 *  summary="Edit existing resource entry by UUID.",
 *  description="Edit resource with data",
 *  operationId="_modelEntry__patch",
 *  tags={"_modelName_: RW context"},
 *  @OA\Parameter(
 *      name="uuid",
 *      in="path",
 *      description="UUID",
 *      required=true,
 *  ),
 *  @OA\RequestBody(
 *      required=true,
 *      description="resource data",
 *      @OA\JsonContent(
 *          format="json",
 *          @OA\Property(property="key", type="string", format="key", example="some_key"),
 *          @OA\Property(property="value", type="string", format="value", example="some_value"),
 *          @OA\Property(property="options", type="string", example="options"),
 *      ),
 *  ),
 *  @OA\Response(
 *      response=200,
 *      description="Success",
 *      @OA\JsonContent(
 *          @OA\Property(property="status", type="integer", example="1"),
 *          @OA\Property(property="data",   type="object", example="{}"),
 *      )
 *  )
 * )
 *
 * @OA\Delete(
 *  path="/api/_model_/{uuid}",
 *  tags={"_modelName_: RW context"},
 *  summary="Delete resource.",
 *  description="Delete resource by UUID.",
 *  @OA\Parameter(
 *      name="uuid",
 *      in="path",
 *      description="UUID",
 *      required=true,
 *  ),
 *  @OA\Response(response="default", description="Delete resource by UUID"),
 * )
 *
 */

class _modelController_ extends Controller
{

    protected $hidden = ['id', 'created_at', 'updated_at','deleted_at'];

    /**
     * Display a listing of the resource.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function index(Request $request)
    {

        $_model_ = _modelName_::whereNotNull('uuid');

        if($request->filter){
            foreach($request->filter as $field_name => $field_value){
                $_model_->where($field_name, $field_value);
            }
        }

        $_model_ = $_model_->get()->each(function ($_modelEntry_) { $_modelEntry_->makeHidden($this->hidden); });

        return [
            "status"    => 1,
            "data"      => $_model_
        ];
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        $request->validate([
            'key'    => 'required',
            'value'           => 'required',
            'options'    => 'required',
        ]);

        $_modelEntry_ = _modelName_::create($request->all());

        return [
            "status"    => 1,
            "data"      => $_modelEntry_->makeHidden($this->hidden)
        ];
    }

    /**
     * Display the specified resource.
     *
     * @param  \App\Models\_modelName_  $_modelEntry_
     * @return \Illuminate\Http\Response
     */
    public function show(_modelName_ $_modelEntry_)
    {
        return [
            "status"    => 1,
            "data"      => $_modelEntry_->makeHidden($this->hidden)
        ];
    }

    /**
     * Update the specified resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  \App\Models\_modelName_  $_modelEntry_
     * @return \Illuminate\Http\Response
     */
    public function update(Request $request, _modelName_ $_modelEntry_)
    {
        $_modelEntry_->update($request->all());
        return [
            "status"    => 1,
            "data"      => $_modelEntry_->makeHidden($this->hidden),
            "msg"       => "_modelName_ updated successfully"
        ];
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  \App\Models\_modelName_  $_modelEntry_
     * @return \Illuminate\Http\Response
     */
    public function destroy(_modelName_ $_modelEntry_)
    {
        $_modelEntry_->delete();
        return [
            "status"    => 1,
            "data"      => $_modelEntry_->makeHidden($this->hidden),
            "msg"       => "_modelName_ deleted successfully"
        ];
    }
}
