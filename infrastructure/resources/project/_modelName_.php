<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class _modelName_ extends Model
{
    use HasFactory;
    protected $fillable = [_fillable_];

    public function getRouteKeyName()
    {
        return 'uuid';
    }
}
