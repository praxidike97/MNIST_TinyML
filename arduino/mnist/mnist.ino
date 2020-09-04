#include <TensorFlowLite.h>

#include "mnist_conv2d_small.h"
#include "mnist_example_image_7.h"

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Global, unnamed namespace. Makes sure that 
// these specific variables are used in this file
// and not variables with the same name from some
// included file.
// Model has to be 'const' otherwise there will be
// a parsing error later on.
namespace{
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  TfLiteTensor* model_output = nullptr;

  int input_length;

  // Allocate space for the model
  constexpr int kTensorArenaSize = 93 * 1024;
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);
  
  Serial.print("Test! \n");

  tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model from the C++ file into a variable
  model = tflite::GetModel(mnist_conv2d_small);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.print("Alles nicht erfolgreich02! \n");
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Just load the operations that are really used in the model
  static tflite::MicroMutableOpResolver<5> micro_op_resolver;
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D, tflite::ops::micro::Register_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D, tflite::ops::micro::Register_MAX_POOL_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED, tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX, tflite::ops::micro::Register_SOFTMAX());
  //micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D, tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE, tflite::ops::micro::Register_RESHAPE());

  // Build interpreter to run the model
  static tflite::MicroInterpreter micro_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &micro_interpreter;

  interpreter->AllocateTensors();

  // Get a pointer to the model input
  model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) ||
      (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != 28) ||
      (model_input->dims->data[2] != 28) ||
      (model_input->dims->data[3] != 1) ||
      (model_input->type != kTfLiteFloat32)) {
        Serial.print("Alles nicht erfolgreich! \n");
        TF_LITE_REPORT_ERROR(error_reporter, "Bad input tensor parameters in model!");
        return;
  }

  input_length = model_input->bytes/sizeof(float);

  // Copy the image in the model input
  memcpy(model_input->data.int8, mnist_example_image_7, model_input->bytes);

  // Invoke the model
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk){
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed! \n");
  }

  model_output = interpreter->output(0);

  //Serial.print("Output: " + itoa(1, number_str, 10) + "\n");
  Serial.print("Output: ");
  Serial.println(1);

  for(int i = 0; i < 10; i++){
    Serial.print("Output ");
    Serial.print(i);
    Serial.print(" ");
    Serial.println(model_output->data.f[i]);
  }

  TF_LITE_REPORT_ERROR(error_reporter,
                       "mnist.  0: %d, 7: %d\n",
                       model_output->data.f[0], model_output->data.f[7]);
  
  Serial.print("Alles erfolgreich! \n");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.print("Test \n");
}
