#!/bin/bash

model="states"
modelEntry="state"
modelName="State"
modelController="StateController"
fillable="'key','value','options'"

cd /project &&
rm /project/database/migrations/*_${model}_table*
migration=$(php artisan make:migration create_${model}_table | grep Migration | awk '{print $3}' | sed -e 's/\[//; s/\]//' )

cat << EOF > /project/database/migrations/${migration}.php
<?php

use Illuminate\\Database\\Migrations\\Migration;
use Illuminate\\Database\\Schema\\Blueprint;
use Illuminate\\Support\\Facades\\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('${model}', function (Blueprint \$table) {
            \$table->id();
            \$table->uuid('uuid');
            \$table->string('key');
            \$table->string('value');
            \$table->string('options');
            \$table->timestamps();

            \$table->unique(array('key', 'value'), '_unique_key' );
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('${model}',function (Blueprint \$table) {
            \$table->dropUnique('_unique_key');
          });
    }
};

EOF

compile_template () {
  cat $1 | \
    sed "s|_model_|${model}|g"                           | \
    sed "s|_modelEntry_|${modelEntry}|g"                 | \
    sed "s|_modelName_|${modelName}|g"                   | \
    sed "s|_modelController_|${modelController}|g"       | \
    sed "s|_fillable_|${fillable}|g"                     | \
    cat
}

cd /project &&

php artisan migrate #:refresh

rm /project/app/Http/Controllers/${modelController}.php
rm /project/app/Models/${modelName}.php

cd / && compile_template _modelName_.php        > /project/app/Models/${modelName}.php
cd / && compile_template _modelController_.php  > /project/app/Http/Controllers/${modelController}.php

#yes | php artisan make:controller ${modelController} --resource --model=${modelName}


echo "use App\Http\Controllers\\${modelController};" >> /project/routes/api.php
echo "Route::resource('${model}', ${modelController}::class);" >> /project/routes/api.php

cd /project && php artisan l5-swagger:generate
cd /project && php artisan vendor:publish --provider "L5Swagger\L5SwaggerServiceProvider"




