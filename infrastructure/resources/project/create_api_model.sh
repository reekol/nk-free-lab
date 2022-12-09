#!/bin/bash

# routes/web.php : \URL::forceScheme('https');
#
# /**
# * @OA\Info(title="Pojects's API", version="0.1")
# */
#

modelEntry="appstorage"

model="${modelEntry}s"

modelName="${modelEntry^}"
modelController="${modelName}Controller"
fillable="'key','value','options'"

compile_template () {
  cat $1 | \
    sed "s|_model_|${model}|g"                           | \
    sed "s|_modelEntry_|${modelEntry}|g"                 | \
    sed "s|_modelName_|${modelName}|g"                   | \
    sed "s|_modelController_|${modelController}|g"       | \
    sed "s|_fillable_|${fillable}|g"                     | \
    cat
}

[ -f "/project/app/Http/Controllers/${modelController}.php" ] && rm /project/app/Http/Controllers/${modelController}.php
[ -f "/project/app/Models/${modelName}.php" ]                 && rm /project/app/Models/${modelName}.php


cd /project
#cd /project && php artisan migrate:rollback
rm /project/database/migrations/*_${model}_table*

migration=$(php artisan make:migration create_${model}_table | grep Migration | awk '{print $3}' | sed -e 's/\[//; s/\]//' )
echo "Migration: create_${model}_table  $migration"

cd / && compile_template _migration_.php        > /project/database/migrations/${migration}.php
cd / && compile_template _modelName_.php        > /project/app/Models/${modelName}.php
cd / && compile_template _modelController_.php  > /project/app/Http/Controllers/${modelController}.php

cd /project && php artisan migrate:refresh

#yes | php artisan make:controller ${modelController} --resource --model=${modelName}

sed -i "/${modelController}/d"                                      /project/routes/api.php
echo "use App\Http\Controllers\\${modelController};"            >>  /project/routes/api.php
echo "Route::resource('${model}', ${modelController}::class);"  >>  /project/routes/api.php

grep -qxF '\URL::forceScheme("https");' /project/routes/web.php || echo '\URL::forceScheme("https");' >> /project/routes/web.php

cd /project && php artisan l5-swagger:generate
cd /project && php artisan vendor:publish --provider "L5Swagger\L5SwaggerServiceProvider"




