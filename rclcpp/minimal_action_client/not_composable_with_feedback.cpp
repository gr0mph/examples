// Copyright 2018 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <inttypes.h>
#include <memory>
#include "example_interfaces/action/fibonacci.hpp"
#include "rclcpp/rclcpp.hpp"
// TODO(jacobperron): Remove this once it is included as part of 'rclcpp.hpp'
#include "rclcpp_action/rclcpp_action.hpp"

using Fibonacci = example_interfaces::action::Fibonacci;
rclcpp::Node::SharedPtr g_node = nullptr;


void feedback_callback(rclcpp_action::ClientGoalHandle<Fibonacci>::SharedPtr, const Fibonacci::Feedback& feedback)
{
  RCLCPP_INFO(
    g_node->get_logger(),
    "Next number in sequence received: %" PRId64,
    feedback.sequence.back());
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  g_node = rclcpp::Node::make_shared("minimal_action_client");
  auto action_client = rclcpp_action::create_client<Fibonacci>(g_node, "fibonacci");

  // Populate a goal
  auto goal_msg = Fibonacci::Goal();
  goal_msg.order = 10;

  RCLCPP_INFO(g_node->get_logger(), "Sending goal");
  // Ask server to achieve some goal and wait until it's accepted
  auto goal_handle_future = action_client->async_send_goal(goal_msg, feedback_callback);

  if (rclcpp::spin_until_future_complete(g_node, goal_handle_future) !=
    rclcpp::executor::FutureReturnCode::SUCCESS)
  {
    RCLCPP_ERROR(g_node->get_logger(), "send goal call failed :(");
    return 1;
  }

  rclcpp_action::ClientGoalHandle<Fibonacci>::SharedPtr goal_handle;
  try {
    goal_handle = goal_handle_future.get();
  } catch (rclcpp_action::exceptions::RejectedGoalError) {
    RCLCPP_ERROR(g_node->get_logger(), "Goal was rejected by server");
    return 1;
  }

  // Wait for the server to be done with the goal
  auto result_future = goal_handle->async_result();

  RCLCPP_INFO(g_node->get_logger(), "Waiting for result");
  if (rclcpp::spin_until_future_complete(g_node, result_future) !=
    rclcpp::executor::FutureReturnCode::SUCCESS)
  {
    RCLCPP_ERROR(g_node->get_logger(), "get result call failed :(");
    return 1;
  }

  // TODO(sloretz) how to check status of result?
  auto result = result_future.get();

  RCLCPP_INFO(g_node->get_logger(), "result received");
  for (auto number : result.sequence)
  {
    RCLCPP_INFO(g_node->get_logger(), "%" PRId64, number);
  }

  action_client.reset();
  g_node.reset();
  rclcpp::shutdown();
  return 0;
}
